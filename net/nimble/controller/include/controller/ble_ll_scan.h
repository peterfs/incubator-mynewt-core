/**
 * Copyright (c) 2015 Runtime Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef H_BLE_LL_SCAN_
#define H_BLE_LL_SCAN_

/* 
 * Configuration items for the number of duplicate advertisers and the
 * number of advertisers from which we have heard a scan response 
 */
#define BLE_LL_SCAN_CFG_NUM_DUP_ADVS         (8)
#define BLE_LL_SCAN_CFG_NUM_SCAN_RSP_ADVS    (8)

/* Dont allow more than 255 of these entries */
#if BLE_LL_SCAN_CFG_NUM_DUP_ADVS > 255
    #error "Cannot have more than 255 duplicate entries!"
#endif
#if BLE_LL_SCAN_CFG_NUM_SCAN_RSP_ADVS > 255
    #error "Cannot have more than 255 scan response entries!"
#endif

/*
 * SCAN_REQ 
 *      -> ScanA    (6 bytes)
 *      -> AdvA     (6 bytes)
 * 
 *  ScanA is the scanners public (TxAdd=0) or random (TxAdd = 1) address
 *  AdvaA is the advertisers public (RxAdd=0) or random (RxAdd=1) address.
 * 
 * Sent by the LL in the Scanning state; received by the LL in the advertising
 * state. The advertising address is the intended recipient of this frame.
 */
#define BLE_SCAN_REQ_LEN                (12)
#define BLE_SCAN_REQ_TXTIME_USECS       (176)

/*
 * SCAN_RSP
 *      -> AdvA         (6 bytes)
 *      -> ScanRspData  (0 - 31 bytes) 
 * 
 *  AdvaA is the advertisers public (TxAdd=0) or random (TxAdd=1) address.
 *  ScanRspData may contain any data from the advertisers host.
 * 
 * Sent by the LL in the advertising state; received by the LL in the
 * scanning state. 
 */
#define BLE_SCAN_RSP_DATA_MAX_LEN       (31)
#define BLE_SCAN_RSP_MIN_LEN            (6)
#define BLE_SCAN_RSP_MAX_LEN            (6 + BLE_SCAN_RSP_DATA_MAX_LEN)

/* Scanning state machine (used when initiating as well) */
struct ble_ll_scan_sm
{
    uint8_t scan_enabled;
    uint8_t scan_type;
    uint8_t own_addr_type;
    uint8_t scan_chan;
    uint8_t scan_filt_policy;
    uint8_t scan_filt_dups;
    uint8_t scan_rsp_pending;
    uint8_t scan_rsp_cons_fails;
    uint8_t scan_rsp_cons_ok;
    uint16_t upper_limit;
    uint16_t backoff_count;
    uint16_t scan_itvl;
    uint16_t scan_window;
    uint32_t scan_win_start_time;
    struct os_mbuf *scan_req_pdu;
    struct os_event scan_win_end_ev;
};

/* Scan types */
#define BLE_SCAN_TYPE_PASSIVE   (BLE_HCI_SCAN_TYPE_PASSIVE)
#define BLE_SCAN_TYPE_ACTIVE    (BLE_HCI_SCAN_TYPE_ACTIVE)
#define BLE_SCAN_TYPE_INITIATE  (2)

/*---- HCI ----*/
/* Set scanning parameters */
int ble_ll_scan_set_scan_params(uint8_t *cmd);

/* Turn scanning on/off */
int ble_ll_scan_set_enable(uint8_t *cmd);

/*--- Controller Internal API ---*/
/* Process scan window end event */
void ble_ll_scan_win_end_proc(void *arg);

/* Initialize the scanner */
void ble_ll_scan_init(void);

/* Called when Link Layer starts to receive a PDU and is in scanning state */
int ble_ll_scan_rx_pdu_start(uint8_t pdu_type, struct os_mbuf *rxpdu);

/* Called when Link Layer has finished receiving a PDU while scanning */
int ble_ll_scan_rx_pdu_end(struct os_mbuf *rxpdu);

/* Process a scan response PDU */
void ble_ll_scan_rx_pdu_proc(uint8_t pdu_type, uint8_t *rxbuf, int8_t rssi,
                             uint8_t flags);

/* Boolean function denoting whether or not the whitelist can be changed */
int ble_ll_scan_can_chg_whitelist(void);

/* Boolean function returning true if scanning enabled */
int ble_ll_scan_enabled(void);

/* Boolean function returns true if whitelist is enabled for scanning */
int ble_ll_scan_whitelist_enabled(void);

/* Initialize the scanner when we start initiating */
struct hci_create_conn;
int ble_ll_scan_initiator_start(struct hci_create_conn *hcc);

/* Returns the PDU allocated by the scanner */
struct os_mbuf *ble_ll_scan_get_pdu(void);

/* Stop the scanning state machine */
void ble_ll_scan_sm_stop(struct ble_ll_scan_sm *scansm);

/* Returns the global scanning state machine */
struct ble_ll_scan_sm *ble_ll_scan_sm_get(void);

#endif /* H_BLE_LL_SCAN_ */