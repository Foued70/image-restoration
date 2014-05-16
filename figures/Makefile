mean  = 0
std   = 30
decay = 30
mid   = 127
mult  = 1

NOISES  = normal laplace
PARAMS  = 10 30 50
FILES  += $(patsubst %, method_%.png, $(PARAMS))
FILES  += $(patsubst %, restored_%.png, $(PARAMS))
TARGETS+= $(patsubst %, normal/%, $(FILES))
TARGETS+= $(patsubst %, laplace/%, $(FILES))


.SECONDARY:

%/noisy.bmp: lena512.bmp
	python ../noise.py -t $(@D) -m $(mean) -s $(std) $< $@

restored_%.bmp: noisy.bmp
	../image-restoration $(*D)/$< $(*F) $@ > /dev/null

method_%.bmp: noisy.bmp restored_%.bmp
	../methodnoise $(*D)/$(word 1, $^) $(word 2, $^) $@ $(mid) $(mult)

%.png: %.bmp
	convert $< $@
	mogrify -density 90 $@

all: $(TARGETS)

.PHONY: clean

clean:
	rm -f $(normals) *.png
