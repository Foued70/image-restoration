im = imread('obk.pgm');

col = 1;
zmax = 1;
imax = 30;
for k = 1:100
	i = 1;
	printf("BOP\n");
	while i < imax
		for z = 1:zmax
			if i >= imax
				break;
			end
			if i > size(im)(2)
				break;
			end
			for c = 1:size(im)(1)
				im(c,i) = mod(i, 2)*255;
			end
			i = i + 1;
		end
	end
	zmax = zmax + 1;
end
