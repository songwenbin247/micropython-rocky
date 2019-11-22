# OpenMV + Robot Arm - By: nxa07175 - 周四 9月 20 2018


import time, utime, sensor, image
from pyb import LED
import esp8266_wifi as es
# For color tracking to work really well you should ideally be in a very, very,
# very, controlled enviroment where the lighting is constant...
red_threshold_01 = (37, 58, 24, 77, -13, 49)
green_threshold_01 = (45, 79, -37, -7, 4, 47)
yellow_threshold_01 = (61, 81, -12, 29, 18, 82)
blue_threshold_01 = (39, 63, -12, 17, -62, -14)

LED(1).on()
LED(2).off()
LED(3).off()

utime.sleep_ms(100)
sensor.reset() # Initialize the camera sensor.
#sensor.set_pixformat(sensor.GRAYSCALE) # use RGB565.
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA) # use QQVGA for speed.
sensor.skip_frames(time = 500) # Let new settings take affect.
sensor.set_auto_gain(False) # must be turned off for color tracking
sensor.set_auto_whitebal(True)
LED(1).off()

es.reset()
print('start to connect to Wifi')
#es.esp8266_station("nxp-ls1043ardb-demo", "NXP123456")
es.esp8266_station("ls1046-gateway", "NXP123456")
LED(2).on()
es.connect_ap()
print('Success to connect to Wifi')
LED(2).off()
LED(3).on()
print('start to connect to TCP')
es.create_tcp_client("192.168.0.8", "12346")
print('Success to Tcp connect')
LED(3).off()

#关闭白平衡。白平衡是默认开启的，在颜色识别中，需要关闭白平衡。
clock = time.clock() # Tracks FPS.
def find_rec_without_draw(img, c_threshold, area_th,color, ri):
    blobs = img.find_blobs([c_threshold], area_threshold=area_th)
    s = ""
    if blobs:
        for b in blobs:
            ri = ri / 2;
            w = int(b[2] / 2 - ri)
            h = int(b[3] / 2 - ri)
            cx = b[5]
            cy = b[6]
            rotation = b[7]
            #print([160-b[5], 120 - b[6]])
            if b[2] > ri*3 or b[3] > ri*3:
                if rotation > 1.55:
                    cx1 = cx + w
                    cy1 = cy - h
                    cx2 = cx - w
                    cy2 = cy + h
                else:
                    cx1 = cx - w
                    cy1 = cy - h
                    cx2 = cx + w
                    cy2 = cy + h
                img.draw_cross(cx1, cy1, color=[0,0,0])
                img.draw_cross(cx2, cy2, color=[0,0,0])
                s = s + ".X:"+str(160-cx1) + ":Y:" + str(120 - cy1) + ".X:"+str(160-cx2) + ":Y:" + str(120 - cy2)
            else:
                s = s + ".X:"+str(160-b[5]) + ":Y:" + str(120 - b[6])
                #img.draw_cross(b[5], b[6],color=[0,0,0])
            img.draw_rectangle(b.rect(), color=color)
    return s


def find_rec(img, c_threshold, area_th,color, ri):
    blobs = img.find_blobs([c_threshold], area_threshold=area_th)
    s = ""
    if blobs:
        for b in blobs:
            ri = ri / 2;
            w = int(b[2] / 2 - ri)
            h = int(b[3] / 2 - ri)
            cx = b[5]
            cy = b[6]
            rotation = b[7]
            if b[2] > 120 or b[3] > 120:
                if rotation > 1.55:
                    cx1 = cx + w
                    cy1 = cy - h
                  #  img.draw_cross(cx1, cy1)
                    cx2 = cx - w
                    cy2 = cy + h
                  #  img.draw_cross(cx2, cy2)
                else:
                    cx1 = cx - w
                    cy1 = cy - h
                  #  img.draw_cross(cx1, cy1)
                    cx2 = cx + w
                    cy2 = cy + h
                  #  img.draw_cross(cx2, cy2)
                s = s + ".X:"+str(160-cx1) + ":Y:" + str(120 - cy1) + ".X:"+str(160-cx2) + ":Y:" + str(120 - cy2)
            else:
               # img.draw_cross(b[5], b[6])
                s = s + ".X:"+str(160-b[5]) + ":Y:" + str(120 - b[6])
            img.draw_rectangle(b.rect(), color=color)
    return s

LED(1).off()
index=0
while(True):

    img1=sensor.snapshot()
    s = "S:" + str(int(index))
    s += "*R" + find_rec_without_draw(img1, red_threshold_01, 1000,(255,0,0), 60)
    s += "/G" + find_rec_without_draw(img1, green_threshold_01, 1000,(0,255,0), 60)
    s += "/Y" + find_rec_without_draw(img1, yellow_threshold_01, 1000,(255,255,0), 60)
    s += "/B" + find_rec_without_draw(img1, blue_threshold_01, 1000,(0,0,255), 55)
    s += "*R" + find_rec(img1, red_threshold_01, 3000,(255,0,0), 90)
    s += "/G" + find_rec(img1, green_threshold_01, 3000,(0,255,0), 90)
    s += "/Y" + find_rec(img1, yellow_threshold_01, 3000,(255,255,0), 90)
    s += "/B" + find_rec(img1, blue_threshold_01, 3000,(0,0,255), 90)
    s += "E\r\n"
  #  print(s)
    utime.sleep_ms(500)
    es.send_a_string(s)

    index = index + 1
    if (index > 10000):
        index = 0
