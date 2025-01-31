/*
 * Copyright (c) 2023  Moddable Tech, Inc.
 *
 *   This file is part of the Moddable SDK.
 *
 *   This work is licensed under the
 *       Creative Commons Attribution 4.0 International License.
 *   To view a copy of this license, visit
 *       <http://creativecommons.org/licenses/by/4.0>.
 *   or send a letter to Creative Commons, PO Box 1866,
 *   Mountain View, CA 94042, USA.
 *
 */

import Proximity from "embedded:sensor/AmbientLight-Proximity/LTR553ALS";
import Timer from "timer";

const sensor = new Proximity({
  sensor: { ...(device.I2C.internal ?? device.I2C.default), io: device.io.SMBus },
});

Timer.repeat(() => {
	const sample = sensor.sample();

	trace(`proximity: ${sample.proximity.distance} / lightmeter: ${sample.lightmeter.illuminance}\n`);
}, 100);
