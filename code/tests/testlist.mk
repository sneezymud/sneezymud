TESTS=	DBTest.h CommTest.h Format.h SString.h Exceptions.h ConvertTo.h \
	Garble.h

TESTS := $(sort $(addprefix tests/, $(TESTS)))


