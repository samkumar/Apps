APPLICATION = openthread_tcp

# If no BOARD is found in the environment, use this default:
#BOARD ?= hamilton

# These are the boards that OpenThread stack has been tested on
BOARD_WHITELIST := hamilton samr21-xpro iotlab-m3 fox iotlab-a8-m3

# This has to be the absolute path to the RIOT base directory:
RIOTBASE = /home/sam/Documents/Research/RIOT-OS

# This is to set the proper programmer for board
PROGRAMMER ?= jlink

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
#CFLAGS += -DDEVELHELP -Wall

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

USEPKG += openthread
OPENTHREAD_TYPE ?= ftd
ifeq ($(OPENTHREAD_TYPE),mtd)
    # MTD: A Minimal Thread Device does not have router functionality
    # compiled in. As a result, it is not necessary to configure the
    # routerrole on an MTD. At the same time, an MTD may or may not be sleepy.
    USEMODULE += openthread-mtd
    USEMODULE += openthread-cli-mtd
    BOARD ?= hamilton
else
    # ftd: A Full Thread Device has router functionality compiled in
    USEMODULE += openthread-ftd
    USEMODULE += openthread-cli-ftd
    #BOARD ?= samr21-xpro
    BOARD ?= hamilton
    CFLAGS += -DCLOCK_USE_PLL=1
endif

#Define PANID, CHANNEL, TXPOWER, and UART_BAUDRATE used by default
OPENTHREAD_PANID ?= 0xbeef
OPENTHREAD_CHANNEL ?= 24
OPENTHREAD_TXPOWER ?= 4
OPENTHREAD_UART_BAUDRATE ?= 115200

CFLAGS += -DOPENTHREAD_PANID=${OPENTHREAD_PANID}
CFLAGS += -DOPENTHREAD_CHANNEL=${OPENTHREAD_CHANNEL}
CFLAGS += -DOPENTHREAD_TXPOWER=${OPENTHREAD_TXPOWER}
CFLAGS += -DOPENTHREAD_UART_BAUDRATE=${OPENTHREAD_UART_BAUDRATE}
CFLAGS += -DOPENTHREAD_SOURCE=${HOSTNAME}
CFLAGS += -DDEVELHELP
#CFLAGS += -DHIGH_DATA_RATE=1

CFLAGS += -DINSTRUMENT_TCP

ifneq (,$(filter hamilton samr21-xpro,$(BOARD)))
  DRIVER := at86rf233
  CFLAGS    += -DAUTO_CSMA_EN=1
endif
ifneq (,$(filter iotlab-m3 fox iotlab-a8-m3,$(BOARD)))
  DRIVER := at86rf231
endif

ifneq (,$(filter at86rf2%,$(DRIVER)))
  FEATURES_REQUIRED += periph_spi periph_gpio
endif

USEMODULE += $(DRIVER)
USEMODULE += schedstatistics

# CPU clock speed
#ifneq (,$(filter hamilton,$(BOARD)))
  CFLAGS    += -DCLOCK_USE_ADAPTIVE=0
  CFLAGS    += -DSENSOR_COMPUTOFFLOAD=1
  CFLAGS    += -DDMAC_ENABLE=0
  USEMODULE += task_sched
  USEMODULE += tcp_freebsd
  USEMODULE += tcp_freebsd_internal
  USEMODULE += tcp_freebsd_internal_cc
  USEMODULE += ipv6_addr
  CFLAGS += -DSOCK_HAS_IPV6
  USEMODULE += posix_sockets
  USEMODULE += sock_tcp_freebsd
#endif

USEMODULE += xtimer
USEMODULE += random
USEMODULE += ps

CFLAGS += -DRADIO_DUTYCYCLE_MONITOR=1
CFLAGS += -DCPU_DUTYCYCLE_MONITOR=1

USEMODULE += rtt_stdio
CFLAGS    += -DRTT_STDIO_DISABLE_STDIN
#CFLAGS    += -DRTT_STDIO_ENABLE_BLOCKING_STDOUT
#CFLAGS    += -DSTDIO_POLL_INTERVAL=5000U
CFLAGS    += -DTHREAD_STACKSIZE_DEFAULT=1650

#required for C++ compiling
CXXEXFLAGS += -fno-rtti
USEMODULE += cpp11-compat

include $(RIOTBASE)/Makefile.include
