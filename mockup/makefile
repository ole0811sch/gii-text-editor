JC = javac
BLDDIR = Build
ROOTPKG = .
JFLAGS = -Xdiags:verbose -d $(BLDDIR)/ --class-path $(ROOTPKG)/
CFLAGS = -I../mockup/includeFX -I../mockup -D MOCKUP -x c -std=c99 -Wall \
	-pedantic -Wno-unknown-pragmas -g

.PHONY: all clean

all: $(BLDDIR)/mockup_gui.jar

clean:
	rm -f -r $(BLDDIR)/* $(ROOTPKG)/me/build/* $(ROOTPKG)/me/mockup_gui/*.class

$(BLDDIR)/mockup_gui.jar: $(BLDDIR)/mockup_gui/RFC.class \
	$(BLDDIR)/mockup_gui/KeyBios.class $(BLDDIR)/mockup_gui/Main.class
	jar -v -c -f $@ -e me.mockup_gui.Main -C $(BLDDIR)/ .

$(BLDDIR)/mockup_gui/RFC.class: $(ROOTPKG)/me/build/mockup_gui/RFC.java \
	$(ROOTPKG)/me/mockup_gui/Main.java $(ROOTPKG)/me/build/mockup_gui/KeyBios.java
	$(JC) $(JFLAGS) $<

$(BLDDIR)/mockup_gui/GUI.class: $(ROOTPKG)/me/mockup_gui/GUI.java
	$(JC) $(JFLAGS) $<

$(BLDDIR)/mockup_gui/KeyBios.class: $(ROOTPKG)/me/build/mockup_gui/KeyBios.java
	$(JC) $(JFLAGS) $<

$(BLDDIR)/mockup_gui/Main.class: $(ROOTPKG)/me/mockup_gui/Main.java
	$(JC) $(JFLAGS) $<

$(ROOTPKG)/me/build/mockup_gui/RFC.java: $(ROOTPKG)/me/mockup_gui/rfc.hjava rfc.h
	mkdir -p $(ROOTPKG)/me/build/mockup_gui/
	cpp $(CFLAGS) -P $< -o $@

$(ROOTPKG)/me/build/mockup_gui/KeyBios.java: $(ROOTPKG)/me/mockup_gui/keybios.hjava
	mkdir -p $(ROOTPKG)/me/build/mockup_gui/
	cpp $(CFLAGS) -P $< -o $@
