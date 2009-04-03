TESTS=	DBTest.h CommTest.h SString.h Exceptions.h ConvertTo.h \
	Garble.h Act.h Tell.h Cast.h

TESTS := $(sort $(addprefix tests/, $(TESTS)))


