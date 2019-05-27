"""
.. module:: capsense

************************
Cypress CapSense Library
************************

This module exposes easy to use functions to handle Cypress CapSense widgets.

.. note:: At the moment the library is tailored for use with CapSense widgets mounted on the PSoC6 WiFi-BT Pioneer Kit (i.e., two buttons and a slider). Tuning and custom configurations are not supported.

    """

@native_c("_capsense_init", 
    [
        "csrc/capsense_ifc.c"
    ],
    [
        "VHAL_CAPSENSE",
        "CapSense_ENTRY_CALLBACK",
        "CapSense_EXIT_CALLBACK"
    ],
    [
    ])
def _init():
    pass

@native_c("_capsense_wait_event", [])
def _wait_event():
    pass

BTN0_RISE = 0
BTN0_FALL = 1
BTN1_RISE = 2
BTN1_FALL = 3
SLIDER_ENTER = 4
SLIDER_LEAVE = 5
SLIDER_LVLCHNG = 6

_callbacks = {}
def _callback_thread():
    while True:
        evt = _wait_event()
        if evt == SLIDER_LEAVE:
            start_pos = _wait_event()
            end_pos   = _wait_event()
        elif evt == SLIDER_ENTER or evt == SLIDER_LVLCHNG:
            cur_pos = _wait_event()

        if evt in _callbacks:
            if evt == SLIDER_LEAVE:
                _callbacks[evt](start_pos, end_pos)
            elif evt == SLIDER_ENTER or evt == SLIDER_LVLCHNG:
                _callbacks[evt](cur_pos)
            else:
                _callbacks[evt]()

def init():
    """
.. function:: init()

    Initializes available CapSense widgets.
    """
    _init()
    thread(_callback_thread)

def on_btn(callback, event=BTN0_RISE):
    """
.. function:: on_btn(callback, event=BTN0_RISE)

    :param callback: callback function
    :param event: event type

    Sets a callback to be called in response to a button event.
    Available events are:

        * ``BTN0_RISE``: triggered on button 0 touch
        * ``BTN0_FALL``: triggered when button 0 press finishes
        * ``BTN1_RISE``: triggered on button 1 touch
        * ``BTN1_FALL``: triggered when button 0 press finishes

    Callbacks are called without additional parameters.

    """
    _callbacks[event] = callback 

def on_slider(callback, event=SLIDER_LEAVE):
    """
.. function:: on_slider(callback, event=SLIDER_LEAVE)

    :param callback: callback function
    :param event: event type

    Sets a callback to be called in response to a slider event.
    Available events are:

        * ``SLIDER_ENTER``: triggered on slider touch
        * ``SLIDER_LEAVE``: triggered when slider press event finishes
        * ``SLIDER_LVLCHNG``: triggered while the slider is pressed, when the slider centroid value changes

    Slider callbacks are called with different parameters, as described below: ::

        # SLIDER_ENTER
        def enter_callback(start_pos):
            print("centroid value:", start_pos)

        # SLIDER_LEAVE
        def leave_callback(start_pos, end_pos):
            print("centroid value on enter and leave:", start_pos, end_pos)

        # SLIDER_LVLCHNG
        def level_callback(cur_pos):
            print("centroid value:", cur_pos)

    """
    _callbacks[event] = callback

@native_c("_capsense_get_centroid", [])
def get_centroid():
    """
.. function:: get_centroid()

    Gets current slider centroid value.

    Returns ``None`` if the slider is currently not touched.
    """
    pass
