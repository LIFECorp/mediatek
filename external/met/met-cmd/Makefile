.PHONY: met-cmd

ifeq ($(KDIR),)
$(error Error! You should execute "make" in parent directory)
endif

met-cmd:
	@if [ -h "$(ALPS)/development/met-cmd" ]; then \
		rm -f $(ALPS)/development/met-cmd; \
	fi;
	@ln -s $$PWD $(ALPS)/development/met-cmd
	cd $(ALPS); ./mk -o=TARGET_BUILD_VARIANT=$(TARGET_BUILD_VARIANT) -t $(PROJECT) mm development/met-cmd; cd -
	cp -f $(ALPS)/out/target/product/$(PROJECT_SHORT)/system/bin/met-cmd $$PWD
ifeq ($(KEEP_LINK_FILE),)
	@rm -f $(ALPS)/development/met-cmd
endif

clean:
	touch *.c *.h
	rm -f met-cmd

