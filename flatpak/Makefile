all: icons flatpak


flatpak: 
	rm -rf build-dir
	flatpak remote-add --if-not-exists --user flathub https://dl.flathub.org/repo/flathub.flatpakrepo
	flatpak-builder --force-clean --user --install-deps-from=flathub --repo=repo --install build-dir nl.dijkewijk.webui-wire.json
	flatpak remove nl.dijkewijk.webui-wire
	flatpak build-bundle repo webui-wire.flatpak nl.dijkewijk.webui-wire --runtime-repo=https://flathub.org/repo/flathub.flatpakrepo

clean:
	rm -rf src
	rm -rf .flatpak-builder
	rm -rf build-dir
	rm -rf repo
	rm -f *.flatpak
	rm -f *.png


icons: 
	convert -background none -density 1000 -resize 512x nl.dijkewijk.webui-wire.svg nl.dijkewijk.webui-wire-512.png
	convert -background none -density 1000 -resize 256x nl.dijkewijk.webui-wire.svg nl.dijkewijk.webui-wire-256.png
	convert -background none -density 1000 -resize 128x nl.dijkewijk.webui-wire.svg nl.dijkewijk.webui-wire-128.png
	convert -background none -density 1000 -resize 64x nl.dijkewijk.webui-wire.svg nl.dijkewijk.webui-wire-64.png
	convert -background none -density 1000 -resize 48x nl.dijkewijk.webui-wire.svg nl.dijkewijk.webui-wire-48.png
	convert -background none -density 1000 -resize 32x nl.dijkewijk.webui-wire.svg nl.dijkewijk.webui-wire-32.png
