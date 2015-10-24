all: sensor actuator controller cloud

sensor: sensor.c common.h
	gcc -o sensor sensor.c

actuator: actuator.c common.h
	gcc -o actuator actuator.c

controller: controller.c common.h
	gcc -o controller controller.c

cloud: cloud.c common.h
	gcc -o cloud cloud.c

clean: 
	rm sensor controller actuator cloud
