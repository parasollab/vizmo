all: clean internal release

internal:
	doxygen Doxyfile.internal; ln -s -T Internal/html/index.html internal.html

release:
	doxygen Doxyfile.release; ln -s -T Release/html/index.html release.html

clean:
	rm -rf Internal Release internal.html release.html
