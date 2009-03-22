TESTS=	DBTest.h CommTest.h Format.h SString.h Exceptions.h ConvertTo.h \
	Garble.h Act.h Tell.h

TESTS := $(sort $(addprefix tests/, $(TESTS)))


