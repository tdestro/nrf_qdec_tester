#!/bin/bash
#JLinkExe -device NRF52832_XXAA -if SWD -speed 12000 -autoconnect 1
# jlinkgdbserver lanches its own rtt server on localhost:19021
JLinkRTTClient
