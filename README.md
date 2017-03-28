# cansniffer-usbuart
A CAN-Bus sniffer based on the Cypress Semiconductor PSoC 5LP.

# Introduction
This project provides a means to monitor the CAN-Bus traffic in a car using only a cheap OBD2 reader (Used solely for its CAN Transceiver) and a Cypress Semiconductior PSoC 5LP prototyping kit (http://www.cypress.com/documentation/development-kitsboards/cy8ckit-059-psoc-5lp-prototyping-kit-onboard-programmer-and). Two wires - CAN data receive and Ground are attached from the OBD2 reader to the PSoC 5LP board.
Monitoring of the data is via a USB-Serial port provided by the PSoC 5LP. The data can be monitored on a regular terminal program such as PuTTY or Tera Term, and ANSI escape codes are used to render the same register in the same part of the screen whenever updated, making it easier to monitor the traffic.
