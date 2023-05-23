CC:=ccache $(CC)
CFLAGS+=-O2

OBJS=$(patsubst %.c,%.o,$(wildcard *.c))

TARGET=data_sender

bin: $(TARGET)

$(OBJS): %.o: %.c
	$(CC) -c $< $(CFLAGS) $(CPPFLAGS) -o $@ -I /usr/local/opt/openssl/include -L /usr/local/opt/openssl/lib -lcrypto

$(TARGET): $(OBJS)
	$(CC) -g $(OBJS) $(LDFLAGS) crc32.o -o $@ -I /usr/local/opt/openssl/include -L /usr/local/opt/openssl/lib -lcrypto

clean:
	$(RM) $(OBJS) $(TARGET)
