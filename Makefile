CC:=ccache $(CC)
CFLAGS+=-O2

OBJS=$(patsubst %.c,%.o,$(wildcard *.c))

TARGET=data_sender

all: $(TARGET)

$(OBJS): %.o: %.c
	$(CC) -c $< $(CFLAGS) $(CPPFLAGS) -o $@

$(TARGET): $(OBJS)
	$(CC) -g $(OBJS) $(LDFLAGS) -o $@ -I /usr/local/opt/openssl/include -L /usr/local/opt/openssl/lib -lcrypto

clean:
	$(RM) $(OBJS) $(TARGET)
