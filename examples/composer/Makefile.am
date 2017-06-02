if DEBUG
AM_CFLAGS = -g3 -O0
AM_CXXFLAGS = -g3 -O0
else
AM_CFLAGS = -O2
AM_CXXFLAGS = -O2
endif

bin_PROGRAMS = composer.wt
composer_wt_SOURCES =  Composer.C Composer.h ComposeExample.C AddresseeEdit.C AddresseeEdit.h  \
	ContactSuggestions.C ContactSuggestions.h Label.C Label.h Option.C Option.h OptionList.C \
	AttachmentEdit.C AttachmentEdit.h Attachment.h ComposeExample.h Contact.h OptionList.h
