# Component Makefile
#
## These include paths would be exported to project level
COMPONENT_ADD_INCLUDEDIRS += MQTTClient

## not be exported to project level
COMPONENT_PRIV_INCLUDEDIRS := MQTTPacket

## This component's src
COMPONENT_SRCS := $(wildcard MQTTClient/*.c) $(wildcard MQTTPacket/*.c)


COMPONENT_OBJS := $(patsubst %.c,%.o, $(COMPONENT_SRCS))

COMPONENT_SRCDIRS := MQTTClient MQTTPacket


##
#CPPFLAGS +=
