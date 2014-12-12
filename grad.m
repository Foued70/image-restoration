im = imread('obk.pgm');

imax = 40;

idx = 1
for i = 0:100
	for j = 1:(floor((imax/(i+1))/2)*2)
		for b = 1:i
			if idx > size(im)(1)
				break;
			end
			for c = 1:size(im)(1)
				im(c,idx) = mod(j, 2) * 255;
			end
			idx = idx + 1
		end
	end
end
