import("stdfaust.lib");
freq = hslider("freq",200,50,1000,0.01);
gain = hslider("gain",0.5,0,1,0.01);

process = os.sawtooth(freq)*gain <: _,_;