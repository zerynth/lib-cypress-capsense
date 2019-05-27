/*
* @Author: lorenzo
* @Date:   2019-02-15 11:45:06
* @Last Modified by:   l.rizzello
* @Last Modified time: 2019-04-12 18:08:29
*/

#include "CapSense.h"

#define ZERYNTH_PRINTF
#include "zerynth.h"

void CapSense_EntryCallback(void) {
    vosEnterIsr();
}

void CapSense_ExitCallback(void) {
    vosExitIsr();
}

extern vos_vectors *ram_vectors;
#define CapSense_ISR__INTC_NUMBER 49u
void CapSense_SsIsrInitialize(cy_israddress address) {
    // executed in critical section, cannot call vosInstallHandler...
    ram_vectors->vectors[CapSense_ISR__INTC_NUMBER] = address;
    vhalIrqEnable(CapSense_ISR__INTC_NUMBER);
}

// SLIDER STATES:
//  - NOT TOUCHED
//  - TOUCHED (start_level, current_level)

typedef enum {
    CAPSENSE_BTN0_RISE = 0,
    CAPSENSE_BTN0_FALL,
    CAPSENSE_BTN1_RISE,
    CAPSENSE_BTN1_FALL,
    CAPSENSE_SLIDER_ENTER,
    CAPSENSE_SLIDER_LEAVE,
    CAPSENSE_SLIDER_LVLCHNG,
} capsense_event_t;

typedef struct _capsense_data {
    VFifo events;
} capsense_data_t;
capsense_data_t capsense_data;

void _capsense_bk_update() {
    uint32_t btn0_state = 0, btn0_nstate = 0, btn1_state = 0, btn1_nstate = 0;
    uint32_t centroid_pos = CapSense_SLIDER_NO_TOUCH, centroid_npos = 0, centroid_touchstart = 0;
    capsense_event_t capsense_event;

    while (1) {
        if (!CapSense_IsBusy()) {
            CapSense_ProcessAllWidgets();

            btn0_nstate = CapSense_IsWidgetActive(CapSense_BUTTON0_WDGT_ID);
            if (btn0_nstate != btn0_state) {
                capsense_event = (btn0_state == 0) ? CAPSENSE_BTN0_RISE : CAPSENSE_BTN0_FALL;
                btn0_state = btn0_nstate;
                vosFifoPut(capsense_data.events, capsense_event);
            }

            btn1_nstate = CapSense_IsWidgetActive(CapSense_BUTTON1_WDGT_ID);
            if (btn1_nstate != btn1_state) {
                capsense_event = (btn1_state == 0) ? CAPSENSE_BTN1_RISE : CAPSENSE_BTN1_FALL;
                btn1_state = btn1_nstate;
                vosFifoPut(capsense_data.events, capsense_event);
            }

            centroid_npos = CapSense_GetCentroidPos(CapSense_LINEARSLIDER0_WDGT_ID);
            if (centroid_pos == CapSense_SLIDER_NO_TOUCH) {
                if (centroid_npos != CapSense_SLIDER_NO_TOUCH) {
                    // entering state TOUCHED, store start pos
                    centroid_touchstart = centroid_npos;
                    centroid_pos = centroid_npos;
                    vosSysLock();
                    vosFifoPutNonBlock(capsense_data.events, CAPSENSE_SLIDER_ENTER);
                    vosFifoPutNonBlock(capsense_data.events, (uint8_t) centroid_pos); // current position
                    vosSysUnlock();
                }
            }
            else {
                if (centroid_npos == CapSense_SLIDER_NO_TOUCH) {
                    // entering state NOT TOUCHED
                    vosSysLock();
                    vosFifoPutNonBlock(capsense_data.events, CAPSENSE_SLIDER_LEAVE);
                    vosFifoPutNonBlock(capsense_data.events, (uint8_t) centroid_touchstart); // start position
                    vosFifoPutNonBlock(capsense_data.events, (uint8_t) centroid_pos); // end position
                    vosSysUnlock();
                }
                else {
                    if (centroid_npos != centroid_pos) {
                        // still on state TOUCHED, but position changed
                        vosSysLock();
                        vosFifoPutNonBlock(capsense_data.events, CAPSENSE_SLIDER_LVLCHNG);
                        vosFifoPutNonBlock(capsense_data.events, (uint8_t) centroid_npos); // current position
                        vosSysUnlock();
                    }
                }
                centroid_pos = centroid_npos;
            }

            CapSense_ScanAllWidgets();
        }
        vosThSleep(TIME_U(10, MILLIS));
    }
}

C_NATIVE(_capsense_init) {
    NATIVE_UNWARN();

    capsense_data.events = vosFifoCreate(20, NULL, NULL, NULL);

    CapSense_Start();
    CapSense_ScanAllWidgets();

    VThread capsense_th_handle = vosThCreate(512, VOS_PRIO_NORMAL, _capsense_bk_update, NULL, NULL);
    vosThResume(capsense_th_handle);

    return ERR_OK;
}

C_NATIVE(_capsense_get_centroid) {
    NATIVE_UNWARN();

    uint32_t centroid_pos = CapSense_GetCentroidPos(CapSense_LINEARSLIDER0_WDGT_ID);
    if (centroid_pos == CapSense_SLIDER_NO_TOUCH) {
        *res = MAKE_NONE();
    }
    else {
        *res = pinteger_new(centroid_pos);
    }
    return ERR_OK;
}

C_NATIVE(_capsense_wait_event) {
    NATIVE_UNWARN();

    RELEASE_GIL();
    uint8_t event = vosFifoGet(capsense_data.events);
    ACQUIRE_GIL();

    *res = PSMALLINT_NEW(event);
    return ERR_OK;
}
