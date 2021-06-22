import math

p=1500-48*4*0.7 # 4 amps loss and 70% efficiency
rpm=int(input("rpm: "))
rps=rpm/60
mm=p/(rps*2*math.pi) #mm stands for torque on the motor
ratio=float(input("gear ratio: "))
mw=mm/ratio #mw stands for torque on the wheel
fh=mw/0.075 #7.5 cm radius
fg=fh/0.2 #0.2 (calculated in radiants) for the sine of 20% steep hill upwards
m=fg/9.81 #carried mass including mass of soap car
v=rps*ratio*0.15*math.pi #in m/s
v=v*3.6
print("maximum mass including car: " + str(m) + "kg")
print("speed " + str(v) + "km/h") 
