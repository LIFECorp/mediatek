
.PHONY: met-tag

ifeq ($(KDIR),)
$(error Error! You should execute "make" in parent directory)
endif

export MET_APP_SRC=$(ALPS)/development/met-tag

all:
	cd $(ALPS); ./mk -o=TARGET_BUILD_VARIANT=$(TARGET_BUILD_VARIANT) -t $(PROJECT) mm development/met-tag/test; cd -
	cp -f $(ALPS)/out/target/product/$(PROJECT_SHORT)/system/bin/mtag* $$PWD

clean:
	touch *.c
	rm -f mtag_test*

