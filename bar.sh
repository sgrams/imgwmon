#  (C) imgwmon bar script, Stanisław J. Grams 2016-2017
#  <sjg@fmdx.pl

#!/bin/bash
ICON_SUNNY="☀"
ICON_RAINY="☂"
SYMBOL_CELSIUS="℃"

SSID=$(iw dev DEVICENAME link | grep SSID | cut -d " " -f 2-)

if [ "${SSID}" = "" ]; then
	STATION_ID=""
else
	STATION_ID=""
fi

PRECIP=$(imgwmon -i ${STATION_ID} -t currentPrecip | cut -d " " -f 2-)
WIND=$(imgwmon -i ${STATION_ID} -t windDirectionTel -d "`date -u +"%Y-%m-%d %H:00"`" | cut -d " " -f -1)

if [[ ${WIND} ]]; then
	if (( $(echo "${WIND} >= 0.0" | bc -l) )) && (( $(echo "120.0 < 22.5" | bc -l) )); then
		ICON_WIND="↑"
	elif (( $(echo "${WIND} >= 22.5" | bc -l) )) && (( $(echo "${WIND} < 67.5" | bc -l) )); then
		ICON_WIND="↗"
	elif (( $(echo "${WIND} >= 67.5" | bc -l) )) && (( $(echo "${WIND} < 112.5" | bc -l) )); then
		ICON_WIND="→"
	elif (( $(echo "${WIND} >= 112.5" | bc -l) )) && (( $(echo "${WIND} < 157.5" | bc -l) )); then
		ICON_WIND="↘"
	elif (( $(echo "${WIND} >= 157.5" | bc -l) )) && (( $(echo "${WIND} < 202.5" | bc -l) )); then
		ICON_WIND="↓"
	elif (( $(echo "${WIND} >= 202.5" | bc -l) )) && (( $(echo "${WIND} < 247.5" | bc -l) )); then
		ICON_WIND="↙"
	elif (( $(echo "${WIND} >= 247.5" | bc -l) )) && (( $(echo "${WIND} < 292.5" | bc -l) )); then
		ICON_WIND="←"
	elif (( $(echo "${WIND} >= 292.5" | bc -l) )) && (( $(echo "${WIND} < 337.5" | bc -l) )); then
		ICON_WIND="↖"
	elif (( $(echo "${WIND} >= 337.5" | bc -l) )) && (( $(echo "${WIND} < 360.0" | bc -l) )); then
		ICON_WIND="↑"
	fi
fi
if [[ ${PRECIP} ]]; then
	if [ "${PRECIP}" = "no precipitation" ]; then
		echo "${ICON_WIND} `imgwmon -i ${STATION_ID} -t windVelocityTel`  ${ICON_SUNNY} `imgwmon -i ${STATION_ID} | cut -d " " -f -1` ${SYMBOL_CELSIUS}"
	else
		echo "${ICON_WIND} `imgwmon -i ${STATION_ID} -t windVelocityTel`  ${ICON_RAINY} `imgwmon -i ${STATION_ID} | cut -d " " -f -1` ${SYMBOL_CELSIUS}"
	fi
fi
