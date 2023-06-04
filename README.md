# XrdpUlalaca 

xrdp module for [麗 -ulalaca-](https://github.com/team-unstablers/ulalaca)

![Screenshot_20220527_171145](https://user-images.githubusercontent.com/964412/170659838-3843d5e9-3372-47f8-940b-4ce183ca5ec9.png)

# NOTE

- **STILL IN HEAVY DEVELOPMENT, NOT SUITABLE FOR PRODUCTION USE YET**
- **This xrdp module DOES NOT work as a standalone module.** (requires `sessionbroker` and `sessionprojector`; you can get these apps from [麗 -ulalaca-](https://github.com/unstabler/ulalaca)).


# INSTALLATION

To install 麗 -Ulalaca- and XrdpUlalaca on your system, Please check our [INSTALLATION GUIDE](https://teamunstablers.notion.site/xrdp-Ulalaca-Getting-started-f82b0c55f0b540a6ac277cc5902361b1).

## BUILD FROM SOURCE
Also, you can build XrdpUlalaca from source code. Check our [experimental homebrew formulas for xrdp](https://github.com/team-unstablers/xrdp-brew-formulas/blob/main/xrdp-tumod/xrdp-git.rb) for build instructions.


# TESTING

### PREPARING

Tests of XrdpUlalaca has not been merged to xrdp upstream yet, so you will need to modify `xrdp_src/configure.ac` to add targets for testing.

```diff
diff --git a/configure.ac b/configure.ac
index 3e3557db..e5154f66 100644
--- a/configure.ac
+++ b/configure.ac
@@ -582,6 +582,7 @@ AC_CONFIG_FILES([
   mc/Makefile
   neutrinordp/Makefile
   ulalaca/Makefile
+  ulalaca/tests/Makefile
   pkgconfig/Makefile
   pkgconfig/xrdp.pc
   pkgconfig/xrdp-uninstalled.pc
```

### RUNNING TESTS

```shell
# run xrdp_src/configure ... 
# To test XrdpUlalaca, you need to build XrdpUlalaca first.
$ cd xrdp_src/ulalaca
$ make -j8

# Then, you can test XrdpUlalaca with the following commands.
$ cd tests
$ make test_ulalaca_xrdp
$ ./test_ulalaca_xrdp
```

# AUTHOR

This software brought to you by [team unstablers](https://unstabler.pl).

### team unstablers

- Gyuhwan Park (@unstabler)


### THANKS TO

- @am0c - 형 앞으로도 계속 하늘에서 저 지켜봐 주세요!! \ ' ')/
