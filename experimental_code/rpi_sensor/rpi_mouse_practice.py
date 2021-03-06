from serial import Serial
import RPi.GPIO as GPIO
from threading import Thread, Lock
import os
import select
import time

os.nice(-20)
# Open serial port and mouse interfaces -> store mouse-data in dicts
sr = Serial("/dev/ttyS0",baudrate=115200)
GPIO.setmode(GPIO.BOARD) #for questions and/or BOARD layout see https://www.raspberrypi-spy.co.uk/2012/06/simple-guide-to-the-rpi-gpio-header-and-pins/
# Dictionaries/structures containing data for each 'SENSOR'
devices = dict(mouse1 = {
	'Name': '1',
	'File': '/dev/input/mouse1',
	'dx': 0,
	'dy': 0},
mouse2 = {
	'Name': '2',
	'File': '/dev/input/mouse0',
	'dx': 0,
	'dy': 0})
lock = Lock()
start = time.time()
#https://johnroach.io/2011/02/16/getting-raw-data-from-a-usb-mouse-in-linux-using-python/
def to_signed(n):
    return n - ((0x80 & n) << 1)

# SENDOUTPUTTHREAD - class for sending data over serial port, subclass of Thread class
def send(pin):
    sr.write(str(pin) +'\n')

def read(dev_file, mouseno):
    while True:
        newdx = newdy = 0
        # Read raw values from mouse device file in linux filesystem
        # https://johnroach.io/2011/02/16/getting-raw-data-from-a-usb-mouse-in-linux-using-python/
        #status, newdx, newdy = tuple(ord(c) for c in dev_file.read(3))
        #help from https://stackoverflow.com/questions/21429369/read-file-with-timeout-in-python as well
        #status, newdx, newdy = tuple(ord(c) for c in os.read(dev_file,3))
        #newdx = to_signed(newdx) 
        #newdy = to_signed(newdy)
        #lock.acquire()
        #devices[mouseno]['dx'] += newdx
        #devices[mouseno]['dy'] += newdy
        #lock.release()


f1 = os.open(devices['mouse1']['File'],os.O_RDONLY)
f2 = os.open(devices['mouse2']['File'],os.O_RDONLY)

thread1 = Thread(group=None, target=read, name='Thread1', args=(f1,'mouse1',))
thread2 = Thread(group=None, target=read, name='Thread2', args=(f2,'mouse2',))

thread1.start()
thread2.start()

GPIO.setup(16, GPIO.IN)
GPIO.setup(15, GPIO.IN)

while True:
    while not GPIO.input(16):
        pass
    start = time.time();
    devices['mouse1']['dx'] = 0
    devices['mouse2']['dx'] = 0
    devices['mouse1']['dy'] = 0
    devices['mouse2']['dx'] = 0
    while GPIO.input(16):
        GPIO.wait_for_edge(15, GPIO.RISING)
        send(time.time()-start)
