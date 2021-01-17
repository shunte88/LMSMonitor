/*
 *      vision.c
 *
 *	(c) 2020-21 Stuart Hunter
 *
 *	TODO:
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	See <http://www.gnu.org/licenses/> to get a copy of the GNU General
 *	Public License.
 *
 */

#include <pthread.h>
#include <stdbool.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>

#include <fcntl.h>
#include <net/if.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include "kiss_fft.h"
#include "visdata.h"
#include "vision.h"

//#include "log.h"

#define VUMETER_DEFAULT_SAMPLE_WINDOW 1024 * 2

static struct vis_t {
    pthread_rwlock_t rwlock;
    uint32_t buf_size;
    uint32_t buf_index;
    bool running;
    uint32_t rate;
    time_t updated;
    int16_t buffer[VIS_BUF_SIZE];
} *vis_mmap = NULL;

static int vis_fd = -1;
static char *mac_address = NULL;

static char *get_mac_address_shmem() {
    struct ifconf ifc;
    struct ifreq *ifr, *ifend;
    struct ifreq ifreq;
    struct ifreq ifs[3];

    uint8_t mac[6] = {0, 0, 0, 0, 0, 0};

    int sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd < 0)
        return NULL;

    ifc.ifc_len = sizeof(ifs);
    ifc.ifc_req = ifs;

    if (ioctl(sd, SIOCGIFCONF, &ifc) == 0) {
        // Get last interface.
        ifend = ifs + (ifc.ifc_len / sizeof(struct ifreq));

        // Loop through interfaces.
        for (ifr = ifc.ifc_req; ifr < ifend; ifr++) {
            if (ifr->ifr_addr.sa_family == AF_INET) {
                strncpy(ifreq.ifr_name, ifr->ifr_name, sizeof(ifreq.ifr_name));
                if (ioctl(sd, SIOCGIFHWADDR, &ifreq) == 0) {
                    memcpy(mac, ifreq.ifr_hwaddr.sa_data, 6);
                    // Leave on first valid address.
                    if (mac[0] + mac[1] + mac[2] != 0)
                        ifr = ifend;
                }
            }
        }
    }

    close(sd);

    char *macaddr = (char *)malloc(18);

    snprintf(macaddr, 18, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1],
             mac[2], mac[3], mac[4], mac[5]);

    return macaddr;
}

static void vissy_reopen(void) {
    char shm_path[40];

    // Unmap memory if it is already mapped.
    if (vis_mmap) {
        munmap(vis_mmap, sizeof(struct vis_t));
        vis_mmap = NULL;
    }

    // Close file access if it exists.
    if (vis_fd != -1) {
        close(vis_fd);
        vis_fd = -1;
    }

    // Get MAC adddress if not already determined.
    if (!mac_address) {
        mac_address = get_mac_address_shmem();
    }

    /*
          The shared memory object is defined by Squeezelite and is identified
          by a name made up from the MAC address.
  */
    sprintf(shm_path, "/squeezelite-%s", mac_address ? mac_address : "");

    // Open shared memory.
    vis_fd = shm_open(shm_path, O_RDWR, 0666);
    if (vis_fd > 0) {
        // Map memory.
        vis_mmap =
            (struct vis_t *)mmap(NULL, sizeof(struct vis_t),
                                 PROT_READ | PROT_WRITE, MAP_SHARED, vis_fd, 0);

        if (vis_mmap == MAP_FAILED) {
            close(vis_fd);
            vis_fd = -1;
            vis_mmap = NULL;
        }
    }
}

void vissy_close(void) {
    if (vis_fd != -1) {
        close(vis_fd);
        vis_fd = -1;
        vis_mmap = NULL;
    }
}

void vissy_check(void) {

    static time_t lastopen = 0;
    time_t now = time(NULL);

    if (!vis_mmap) {
        if (now - lastopen > 5) {
            vissy_reopen();
            lastopen = now;
        }
        if (!vis_mmap)
            return;
    }

    pthread_rwlock_rdlock(&vis_mmap->rwlock);

    bool running = vis_mmap->running;

    if (running && (now - vis_mmap->updated > 5)) {
        pthread_rwlock_unlock(&vis_mmap->rwlock);
        vissy_reopen();
        lastopen = now;
    } else {
        pthread_rwlock_unlock(&vis_mmap->rwlock);
    }
}

static void vissy_lock(void) {
    if (!vis_mmap)
        return;
    pthread_rwlock_rdlock(&vis_mmap->rwlock);
}

static void vissy_unlock(void) {
    if (!vis_mmap)
        return;
    pthread_rwlock_unlock(&vis_mmap->rwlock);
}

static bool vissy_is_playing(void) {
    if (!vis_mmap)
        return false;
    return vis_mmap->running;
}

uint32_t vissy_get_rate(void) {
    if (!vis_mmap)
        return 0;
    return vis_mmap->rate;
}

static int16_t *vissy_get_buffer(void) {
    if (!vis_mmap)
        return NULL;
    return vis_mmap->buffer;
}

static uint32_t vissy_get_buffer_len(void) {
    if (!vis_mmap)
        return 0;
    return vis_mmap->buf_size;
}

static uint32_t vissy_get_buffer_idx(void) {
    if (!vis_mmap)
        return 0;
    return vis_mmap->buf_index;
}

void vissy_meter_init(struct vissy_meter_t *vissy_meter) {

    int l2int = 0;
    int shiftsubbands;

    // Approximate the number of subbands we'll display based
    // on the width available and the size of the histogram bars.
    vissy_meter->num_subbands =
        vissy_meter->channel_width[0] / vissy_meter->bar_size[0];

    // Calculate the integer component of the log2 of the num_subbands
    l2int = 0;
    shiftsubbands = vissy_meter->num_subbands;
    while (shiftsubbands != 1) {
        l2int++;
        shiftsubbands >>= 1;
    }

    // The actual number of subbands is the largest power
    // of 2 smaller than the specified width.
    vissy_meter->num_subbands = 1L << l2int;

    // In the case where we're going to clip the higher frequency
    // bands, we choose the next highest power of 2.
    if (vissy_meter->clip_subbands[0])
        vissy_meter->num_subbands <<= 1;

    // The number of histogram bars we'll display is nominally
    // the number of subbands we'll compute.
    vissy_meter->num_bars[0] = vissy_meter->num_subbands;

    // Though we may have to compute more subbands to meet
    // a minimum and average them into the histogram bars.
    if (vissy_meter->num_subbands < MIN_SUBBANDS) {
        vissy_meter->subbands_in_bar[0] =
            MIN_SUBBANDS / vissy_meter->num_subbands;
        vissy_meter->num_subbands = MIN_SUBBANDS;
    } else {
        vissy_meter->subbands_in_bar[0] = 1;
    }

    // If we're clipping off the higher subbands we cut down
    // the actual number of bars based on the width available.
    if (vissy_meter->clip_subbands[0]) {
        vissy_meter->num_bars[0] =
            vissy_meter->channel_width[0] / vissy_meter->bar_size[0];
    }

    // Since we now have a fixed number of subbands, we choose
    // values for the second channel based on these.
    if (!vissy_meter->is_mono) {
        vissy_meter->num_bars[1] =
            vissy_meter->channel_width[1] / vissy_meter->bar_size[1];
        vissy_meter->subbands_in_bar[1] = 1;
        // If we have enough space for all the subbands, great.
        if (vissy_meter->num_bars[1] > vissy_meter->num_subbands) {
            vissy_meter->num_bars[1] = vissy_meter->num_subbands;

            // If not, we find the largest factor of the
            // number of subbands that we can show.
        } else if (!vissy_meter->clip_subbands[1]) {
            int s = vissy_meter->num_subbands;
            vissy_meter->subbands_in_bar[1] = 1;
            while (s > vissy_meter->num_bars[1]) {
                s >>= 1;
                vissy_meter->subbands_in_bar[1]++;
            }
            vissy_meter->num_bars[1] = s;
        }
    }

    // Calculate the number of samples we'll need to send in as
    // input to the FFT. If we're halving the bandwidth (by
    // averaging adjacent samples), we're going to need twice
    // as many.
    vissy_meter->sample_window = vissy_meter->num_subbands * 2 * X_SCALE_LOG;

    if (vissy_meter->sample_window < MIN_FFT_INPUT_SAMPLES) {
        vissy_meter->num_windows =
            MIN_FFT_INPUT_SAMPLES / vissy_meter->sample_window;
    } else {
        vissy_meter->num_windows = 1;
    }

    if (vissy_meter->cfg) {
        free(vissy_meter->cfg);
        vissy_meter->cfg = NULL;
    }

    if (!vissy_meter->cfg) {
        double const1;
        double const2;
        int w;

        double freq_sum;
        double scale_db;
        double e;

        int s;

        vissy_meter->cfg =
            kiss_fft_alloc(vissy_meter->sample_window, 0, NULL, NULL);

        const1 = 0.54;
        const2 = 0.46;
        for (w = 0; w < vissy_meter->sample_window; w++) {
            const double twopi = 6.283185307179586476925286766;
            vissy_meter->filter_window[w] =
                const1 - (const2 * cos(twopi * (double)w /
                                       (double)vissy_meter->sample_window));
        }

        // Compute the preemphasis
        freq_sum = 0;
        scale_db = 0;

        // compute the decade scale
        e = log(vissy_meter->num_subbands * X_SCALE_LOG) /
            log(vissy_meter->num_subbands);

        vissy_meter->decade_idx[0] = 1;
        for (s = 0; s < vissy_meter->num_subbands - 1; s++) {
            vissy_meter->decade_idx[s + 1] = pow(s + 1, e) + 1;
            vissy_meter->decade_len[s] =
                vissy_meter->decade_idx[s + 1] - vissy_meter->decade_idx[s];

            while (freq_sum > 1) {
                freq_sum -= 1;
                scale_db += 1.2; // 1.2 dB per kHz
            }

            if (scale_db != 0) {
                vissy_meter->preemphasis[s] = pow(10, (scale_db / 10.0));
            } else {
                vissy_meter->preemphasis[s] = 1;
            }
            freq_sum += (vissy_get_rate() / 1000) /
                        ((float)(vissy_meter->num_subbands * X_SCALE_LOG) /
                         vissy_meter->decade_len[s]);
        }
        vissy_meter->decade_len[s] = (vissy_meter->num_subbands * X_SCALE_LOG) -
                                     vissy_meter->decade_idx[s] + 1;
        vissy_meter->preemphasis[s] = pow(10, (scale_db / 10.0));
    }
}

bool stashvissy_meter_calc(struct vissy_meter_t *vissy_meter) {

    int16_t *ptr;
    int16_t sample;
    uint8_t channel;
    uint64_t sample_sqr[METER_CHANNELS];
    uint64_t sample_sum[METER_CHANNELS];
    uint64_t sample_rms[METER_CHANNELS];
    size_t i, num_samples, samples_until_wrap;

    int MIN_DB = 20 * log10(1.0 / vissy_meter->reference);
    int MAX_DB = 0;

    int offs;

    num_samples = VUMETER_DEFAULT_SAMPLE_WINDOW;

    vissy_check(); // safe

    for (channel = 0; channel < METER_CHANNELS; channel++) {
        vissy_meter->sample_accum[channel] = 0;
        for (i = 0; i < MAX_SUBBANDS; i++) {
            vissy_meter->sample_bin_chan[channel][i] = 0;
        }
    }

    for (i = 0; i < (size_t)(2 * vissy_meter->num_subbands); i++)
        vissy_meter->avg_power[i] = 0;

    bool ret = false;

    vissy_lock();
    if (vissy_is_playing()) {

        offs = vissy_get_buffer_idx() -
               (num_samples * 2); // works for FFT with 1 window
        while (offs < 0)
            offs += vissy_get_buffer_len();

        ptr = vissy_get_buffer() + offs;
        samples_until_wrap = vissy_get_buffer_len() - offs;

        for (i = 0; i < num_samples; i++) {
            for (channel = 0; channel < METER_CHANNELS; channel++) {
                sample = (*ptr++) >> 7;
                sample_sqr[channel] = sample * sample;
                sample_sum[channel] += abs(sample);
                vissy_meter->sample_accum[channel] += sample_sqr[channel];
            }
            samples_until_wrap -= 2;
            if (samples_until_wrap <= 0) {
                ptr = vissy_get_buffer();
                samples_until_wrap = vissy_get_buffer_len();
            }
        }
        ret = true;
    }
    vissy_unlock();

    for (channel = 0; channel < METER_CHANNELS; channel++) {

        vissy_meter->sample_accum[channel] /= num_samples;

        float avg = sample_sum[channel] / num_samples;
        vissy_meter->dB[channel] =
            20 * log10((float)avg / (float)vissy_meter->reference);

        sample_rms[channel] = round(sqrt(sample_sqr[channel]));
        vissy_meter->dBfs[channel] = 20 * log10((float)sample_rms[channel] /
                                                (float)vissy_meter->reference);
        if (vissy_meter->dBfs[channel] < vissy_meter->floor)
            vissy_meter->dBfs[channel] = vissy_meter->floor;

        vissy_meter->linear[channel] =
            abs((int)((vissy_meter->dB[channel] - MIN_DB) / (MAX_DB - MIN_DB)) *
                100);
        if (vissy_meter->linear[channel] > 100)
            vissy_meter->linear[channel] = 100;
    }

    return ret;
}

bool vissy_meter_calc(struct vissy_meter_t *vissy_meter, bool samode) {

    int16_t *ptr;
    int16_t sample;
    uint8_t channel;
    uint64_t sample_sqr[METER_CHANNELS];
    uint64_t sample_sum[METER_CHANNELS];
    uint64_t sample_rms[METER_CHANNELS];
    size_t i, num_samples, samples_until_wrap;

    int MIN_DB = 20 * log10(1.0 / vissy_meter->reference);
    int MAX_DB = 0;

    int offs;

    num_samples = VUMETER_DEFAULT_SAMPLE_WINDOW;

    vissy_check();

    for (channel = 0; channel < METER_CHANNELS; channel++) {
        vissy_meter->sample_accum[channel] = 0;
        vissy_meter->linear[channel] = 0;
        vissy_meter->dB[channel] = -1000;
        vissy_meter->dBfs[channel] = -1000;
        vissy_meter->rms_scale[channel] = 0;
        for (i = 0; i < MAX_SUBBANDS; i++) {
            vissy_meter->sample_bin_chan[channel][i] = 0;
        }
    }

    for (i = 0; i < (size_t)(2 * vissy_meter->num_subbands); i++) {
        vissy_meter->avg_power[i] = 0;
    }

    kiss_fft_cpx fin_buf[MAX_SAMPLE_WINDOW];
    kiss_fft_cpx fout_buf[MAX_SAMPLE_WINDOW];
    bool ret = false;

    vissy_lock();
    if (vissy_is_playing()) {

        ret = true;

        offs = vissy_get_buffer_idx() -
               (num_samples * 2); // works for FFT with 1 window
        while (offs < 0)
            offs += vissy_get_buffer_len();

        ptr = vissy_get_buffer() + offs;
        samples_until_wrap = vissy_get_buffer_len() - offs;

        for (i = 0; i < num_samples; i++) {
            for (channel = 0; channel < METER_CHANNELS; channel++) {
                if (!samode)
                    sample = (*ptr++) >> 8;
                else
                    sample = (*ptr++) >> 7;

                sample_sqr[channel] = sample * sample;
                sample_sum[channel] += ((sample < 0) ? -sample : sample);
                vissy_meter->sample_accum[channel] += sample_sqr[channel];
                if (0 == channel) {
                    fin_buf[i].r =
                        (float)(vissy_meter->filter_window[i] * sample);
                } else {
                    fin_buf[i].i =
                        (float)(vissy_meter->filter_window[i] * sample);
                }
            }

            samples_until_wrap -= 2;
            if (samples_until_wrap <= 0) {
                ptr = vissy_get_buffer();
                samples_until_wrap = vissy_get_buffer_len();
            }
        }
    }
    vissy_unlock();

    if (ret) {

        if (samode) // SA mode
        {

            kiss_fft(vissy_meter->cfg, fin_buf, fout_buf);

            int avg_ptr = 0;

            // Extract the two separate frequency domain signals
            // and keep track of the power per bin.
            for (int s = 0; s < vissy_meter->num_subbands; s++) {

                kiss_fft_cpx ck, cnk;

                float kr = 0, ki = 0;
                int x;

                for (x = vissy_meter->decade_idx[s];
                     x <
                     vissy_meter->decade_idx[s] + vissy_meter->decade_len[s];
                     x++) {
                    ck = fout_buf[x];
                    cnk = fout_buf[vissy_meter->sample_window - x];

                    kr = (ck.r + cnk.r) / 2;
                    ki = (ck.i - cnk.i) / 2;

                    vissy_meter->avg_power[avg_ptr] +=
                        (kr * kr + ki * ki) / vissy_meter->num_windows;

                    kr = (cnk.i + ck.i) / 2;
                    ki = (cnk.r - ck.r) / 2;

                    vissy_meter->avg_power[avg_ptr + 1] +=
                        (kr * kr + ki * ki) / vissy_meter->num_windows;
                }

                vissy_meter->avg_power[avg_ptr] /= vissy_meter->decade_len[s];
                vissy_meter->avg_power[avg_ptr + 1] /=
                    vissy_meter->decade_len[s];

                avg_ptr += 2;
            }

            int pre_ptr = 0;

            avg_ptr = 0;
            for (int p = 0; p < vissy_meter->num_subbands; p++) {

                long product = (long)(vissy_meter->avg_power[avg_ptr] *
                                      vissy_meter->preemphasis[pre_ptr]);

                product >>= 16;
                vissy_meter->avg_power[avg_ptr++] = (int)product;

                product = (long)(vissy_meter->avg_power[avg_ptr] *
                                 vissy_meter->preemphasis[pre_ptr]);

                product >>= 16;
                vissy_meter->avg_power[avg_ptr++] = (int)product;

                pre_ptr++;
            }
        }

        for (channel = 0; channel < METER_CHANNELS; channel++) {

            if (samode) {
                int power_sum = 0;
                int in_bar = 0;
                int curr_bar = 0;

                int avg_ptr = (0 == channel) ? 0 : 1;

                int s;

                for (s = 0; s < vissy_meter->num_subbands; s++) {
                    // Average out the power for all subbands represented
                    // by a bar.
                    power_sum += vissy_meter->avg_power[avg_ptr] /
                                 vissy_meter->subbands_in_bar[channel];

                    if (++in_bar == vissy_meter->subbands_in_bar[channel]) {
                        int val;
                        int i;

                        power_sum <<= 6; // FIXME scaling - 6 is height ???

                        val = 0;
                        for (i = 31; i > 0; i--) {
                            if (power_sum >= vissy_meter->power_map[i]) {
                                val = i;
                                break;
                            }
                        }

                        vissy_meter->sample_bin_chan[channel][curr_bar++] = val;

                        if (curr_bar == vissy_meter->num_bars[channel]) {
                            break;
                        }

                        in_bar = 0;
                        power_sum = 0;
                    }
                    avg_ptr += 2;
                }
            }

            vissy_meter->sample_accum[channel] /= num_samples;

            float avg = sample_sum[channel] / num_samples;
            vissy_meter->dB[channel] =
                20 * log10((float)avg / (float)vissy_meter->reference);

            sample_rms[channel] = round(sqrt(sample_sqr[channel]));
            vissy_meter->dBfs[channel] =
                20 * log10((float)sample_rms[channel] /
                           (float)vissy_meter->reference);
            if (vissy_meter->dBfs[channel] < vissy_meter->floor)
                vissy_meter->dBfs[channel] = vissy_meter->floor;

            vissy_meter->linear[channel] = abs(
                (int)((vissy_meter->dB[channel] - MIN_DB) / (MAX_DB - MIN_DB)) *
                100);
            if (vissy_meter->linear[channel] > 100)
                vissy_meter->linear[channel] = 100;
        }
    }

    return ret;
}

// end.
