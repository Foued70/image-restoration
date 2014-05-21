mean  = 0
std   = 30
decay = 30
mid   = 127
mult  = 10

NOISES  = normal laplace
NPARAMS = 10 30 50
LPARAMS = 0.3 0.7 1
NFILES  += $(patsubst %, method_%.png, $(NPARAMS))
NFILES  += $(patsubst %, restored_%.png, $(NPARAMS))
LFILES  += $(patsubst %, method_%.png, $(LPARAMS))
LFILES  += $(patsubst %, restored_%.png, $(LPARAMS))
FILES   += noisy.png
TARGETS += $(patsubst %, normal/%, $(NFILES))
TARGETS += $(patsubst %, laplace/%, $(LFILES))


.SECONDARY:

%/noisy.bmp: lena512.bmp
	python ../noise.py -t $(@D) -m $(mean) -s $(std) $< $@

restored_%.bmp: noisy.bmp

	if   [ $(*D) = "laplace" ]; then \
		../image-restoration -b $(*F) -p 1 $(*D)/$< $@ > /dev/null; \
	else \
		../image-restoration -b $(*F) -p 2 $(*D)/$< $@ > /dev/null; \
	fi

method_%.bmp: noisy.bmp restored_%.bmp
	../methodnoise $(*D)/$(word 1, $^) $(word 2, $^) $@ $(mid) $(mult)

%.png: %.bmp
	convert -colorspace gray -gamma 2.2 $< $@
	mogrify -density 90 $@

all: $(TARGETS)

.PHONY: clean

clean:
	rm -f $(TARGETS) *.png 

