
.PHONY: met-tag

ifeq ($(KDIR),)
$(error Error! You should execute "make" in parent directory)
endif

met-tag:
	@if [ -h "$(ALPS)/development/met-tag" ]; then \
		rm -f $(ALPS)/development/met-tag; \
	fi;
	@ln -s $$PWD $(ALPS)/development/met-tag
	cd $(ALPS); ./mk -o=TARGET_BUILD_VARIANT=$(TARGET_BUILD_VARIANT) -t $(PROJECT) mm development/met-tag; cd -
	cp -f $(ALPS)/out/target/product/$(PROJECT_SHORT)/system/lib/libmet-tag.so $$PWD
	$(MAKE) -C test
ifeq ($(KEEP_LINK_FILE),)
	@rm -f $(ALPS)/development/met-tag
endif

clean:
	touch *.c *.h
	rm -f libmet-tag.so
	$(MAKE) -C test clean

