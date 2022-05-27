# ulalaca-xrdp 

![Screenshot_20220527_171145](https://user-images.githubusercontent.com/964412/170659838-3843d5e9-3372-47f8-940b-4ce183ca5ec9.png)

# NOTE

- **STILL IN HEAVY DEVELOPMENT, NOT SUITABLE FOR PRODUCTION USE YET**
- This xrdp module requires `sessionbroker` and `sessionprojector`, you can get these apps from [麗 -ulalaca-](https://github.com/unstabler/ulalaca).

# INSTALLATION
1. fetch xrdp source code
```shell
$ git clone https://github.com/neutrinolabs/xrdp.git xrdp
$ cd xrdp
$ git checkout devel
```

2. add ulalaca-xrdp into xrdp source tree

3. apply patch
```shell
$ patch -p1 < ulalaca/xrdp-automake.patch
$ patch -p1 < ulalaca/xrdp-encoder-force-use-bgra.patch
```

4. build and install
```shell
$ ./bootstrap
$ ./configure --enable-pixman PKG_CONFIG_PATH=/usr/local/opt/openssl/lib/pkgconfig:/usr/local/opt/libjpeg-turbo/lib/pkgconf
$ make -j8 
$ make install
```

5. edit /etc/xrdp/xrdp.ini
```diff
 ; Section name to use for automatic login if the client sends username
 ; and password. If empty, the domain name sent by the client is used.
 ; If empty and no domain name is given, the first suitable section in
 ; this file will be used.
 autorun=

 allow_channels=true
 allow_multimon=true
 bitmap_cache=true
-bitmap_compression=true
+bitmap_compression=false
-bulk_compression=true
+bulk_compression=false
 #hidelogwindow=true
 max_bpp=32
 new_cursors=true
 
 ; ...
 
+[Ulalaca]
+name=Ulalaca
+lib=libulalaca.dylib
+username=ask
+password=ask
```

# AUTHOR

This software brought to you by [team unstablers](https://unstabler.pl).

### team unstablers

- Gyuhwan Park (@unstabler)


### THANKS TO

- @am0c - 형 앞으로도 계속 하늘에서 저 지켜봐 주세요!! \ ' ')/
