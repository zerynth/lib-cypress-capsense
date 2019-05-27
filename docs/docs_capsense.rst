.. module:: capsense

************************
Cypress CapSense Library
************************

This module exposes easy to use functions to handle Cypress CapSense widgets.

.. note:: At the moment the library is tailored for use with CapSense widgets mounted on the PSoC6 WiFi-BT Pioneer Kit (i.e., two buttons and a slider). Tuning and custom configurations are not supported.

    
.. function:: init()

    Initializes available CapSense widgets.
    
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

    
.. function:: get_centroid()

    Gets current slider centroid value.

    Returns ``None`` if the slider is currently not touched.
    
