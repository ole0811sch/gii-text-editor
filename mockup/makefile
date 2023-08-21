JC = javac
BLDDIR = Build
ROOTPKG = me
JFLAGS = -Xdiags:verbose -d $(BLDDIR)/ --class-path $(ROOTPKG)/

.PHONY: all clean

all: $(BLDDIR)/mockup_gui.jar

clean:
	rm -f -r $(BLDDIR)/* $(ROOTPKG)/build/* $(ROOTPKG)/mockup_gui/*.class
$(BLDDIR)/mockup_gui.jar: $(BLDDIR)/mockup_gui/RFC.class $(BLDDIR)/mockup_gui/KeyBios.class $(BLDDIR)/mockup_gui/Main.class
	jar -v -c -f $@ -e mockup_gui.Main -C $(BLDDIR)/ .

$(BLDDIR)/mockup_gui/RFC.class: $(ROOTPKG)/build/mockup_gui/RFC.java $(ROOTPKG)/mockup_gui/Main.java $(ROOTPKG)/build/mockup_gui/KeyBios.java
	$(JC) $(JFLAGS) $<

$(BLDDIR)/mockup_gui/GUI.class: $(ROOTPKG)/mockup_gui/GUI.java
	$(JC) $(JFLAGS) $<

$(BLDDIR)/mockup_gui/KeyBios.class: $(ROOTPKG)/build/mockup_gui/KeyBios.java
	$(JC) $(JFLAGS) $<

$(BLDDIR)/mockup_gui/Main.class: $(ROOTPKG)/mockup_gui/Main.java
	$(JC) $(JFLAGS) $<

$(ROOTPKG)/build/mockup_gui/RFC.java: $(ROOTPKG)/mockup_gui/rfc.hjava
	mkdir -p $(ROOTPKG)/build/mockup_gui/
	cpp -P $< -o $@

$(ROOTPKG)/build/mockup_gui/KeyBios.java: $(ROOTPKG)/mockup_gui/keybios.hjava 
	mkdir -p $(ROOTPKG)/build/mockup_gui/
	cpp -P $< -o $@
