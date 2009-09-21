/*-
 * Copyright (c) 2001 Atsushi Onoe
 * Copyright (c) 2002-2004 Sam Leffler, Errno Consulting
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * IEEE 802.11 input handling.
 */

#include "a_config.h"
#include "athdefs.h"
#include "a_types.h"
#include "a_osapi.h"
#include <wmi.h>
#include <ieee80211.h>
#include <wlan_api.h>

#define IEEE80211_VERIFY_LENGTH(_len, _minlen) do {         \
    if ((_len) < (_minlen)) {                   \
        return A_EINVAL;                         \
    }                               \
} while (0)

#define IEEE80211_VERIFY_ELEMENT(__elem, __maxlen) do {         \
    if ((__elem) == NULL) {                     \
        return A_EINVAL;                         \
    }                               \
    if ((__elem)[1] > (__maxlen)) {                 \
        return A_EINVAL;                         \
    }                               \
} while (0)


/* unaligned little endian access */
#define LE_READ_2(p)                            \
    ((A_UINT16)                            \
     ((((A_UINT8 *)(p))[0]      ) | (((A_UINT8 *)(p))[1] <<  8)))

#define LE_READ_4(p)                            \
    ((A_UINT32)                            \
     ((((A_UINT8 *)(p))[0]      ) | (((A_UINT8 *)(p))[1] <<  8) | \
      (((A_UINT8 *)(p))[2] << 16) | (((A_UINT8 *)(p))[3] << 24)))


static int __inline
iswpaoui(const A_UINT8 *frm)
{
    return frm[1] > 3 && LE_READ_4(frm+2) == ((WPA_OUI_TYPE<<24)|WPA_OUI);
}

static int __inline
iswmmoui(const A_UINT8 *frm)
{
    return frm[1] > 3 && LE_READ_4(frm+2) == ((WMM_OUI_TYPE<<24)|WMM_OUI);
}

static int __inline
iswmmparam(const A_UINT8 *frm)
{
    return frm[1] > 5 && frm[6] == WMM_PARAM_OUI_SUBTYPE;
}

static int __inline
iswmminfo(const A_UINT8 *frm)
{
    return frm[1] > 5 && frm[6] == WMM_INFO_OUI_SUBTYPE;
}

static int __inline
isatherosoui(const A_UINT8 *frm)
{
    return frm[1] > 3 && LE_READ_4(frm+2) == ((ATH_OUI_TYPE<<24)|ATH_OUI);
}

static int __inline
iswscoui(const A_UINT8 *frm)
{
    return frm[1] > 3 && LE_READ_4(frm+2) == ((0x04<<24)|WPA_OUI);
}

A_STATUS
wlan_parse_beacon(A_UINT8 *buf, int framelen, struct ieee80211_common_ie *cie)
{
    A_UINT8 *frm, *efrm;

    frm = buf;
    efrm = (A_UINT8 *) (frm + framelen);

    /*
     * beacon/probe response frame format
     *  [8] time stamp
     *  [2] beacon interval
     *  [2] capability information
     *  [tlv] ssid
     *  [tlv] supported rates
     *  [tlv] country information
     *  [tlv] parameter set (FH/DS)
     *  [tlv] erp information
     *  [tlv] extended supported rates
     *  [tlv] WMM
     *  [tlv] WPA or RSN
     *  [tlv] Atheros Advanced Capabilities
     */
    IEEE80211_VERIFY_LENGTH(efrm - frm, 12);
    A_MEMZERO(cie, sizeof(*cie));

    cie->ie_tstamp = frm; frm += 8;
    cie->ie_beaconInt = A_LE2CPU16(*(A_UINT16 *)frm);  frm += 2;
    cie->ie_capInfo = A_LE2CPU16(*(A_UINT16 *)frm);  frm += 2;
    cie->ie_chan = 0;

    while (frm < efrm) {
        switch (*frm) {
        case IEEE80211_ELEMID_SSID:
            cie->ie_ssid = frm;
            break;
        case IEEE80211_ELEMID_RATES:
            cie->ie_rates = frm;
            break;
        case IEEE80211_ELEMID_COUNTRY:
            cie->ie_country = frm;
            break;
        case IEEE80211_ELEMID_FHPARMS:
            break;
        case IEEE80211_ELEMID_DSPARMS:
            cie->ie_chan = frm[2];
            break;
        case IEEE80211_ELEMID_TIM:
            cie->ie_tim = frm;
            break;
        case IEEE80211_ELEMID_IBSSPARMS:
            break;
        case IEEE80211_ELEMID_XRATES:
            cie->ie_xrates = frm;
            break;
        case IEEE80211_ELEMID_ERP:
            if (frm[1] != 1) {
                //A_PRINTF("Discarding ERP Element - Bad Len\n");
                return A_EINVAL;
            }
            cie->ie_erp = frm[2];
            break;
        case IEEE80211_ELEMID_RSN:
            cie->ie_rsn = frm;
            break;
        case IEEE80211_ELEMID_VENDOR:
            if (iswpaoui(frm)) {
                cie->ie_wpa = frm;
            } else if (iswmmoui(frm)) {
                cie->ie_wmm = frm;
            } else if (isatherosoui(frm)) {
                cie->ie_ath = frm;
            } else if(iswscoui(frm)) {
                cie->ie_wsc = frm;
            }
            break;
        default:
            break;
        }
        frm += frm[1] + 2;
    }
    IEEE80211_VERIFY_ELEMENT(cie->ie_rates, IEEE80211_RATE_MAXSIZE);
    IEEE80211_VERIFY_ELEMENT(cie->ie_ssid, IEEE80211_NWID_LEN);

    return A_OK;
}
