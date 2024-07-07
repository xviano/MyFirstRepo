NOTES:

* These tools require Windows XP or later.

* The output format of md4/md5/sha1sum conforms to the "standard" output format
  used by the GNU coreutils (lowercase hash, space, binary flag, file name).

* The output format of crc32sum conforms to the SFV format.

* Wildcards may be used; for example, "md4sum *.exe *.dll" will output the MD4
  hashes of every file with the .exe or .dll extension in the current directory.

* The -c command line option is not implemented; I may add it in a future
  release, but it is not a priority since I personally never use it.

* The -r command line switch may be used to walk the directory tree; for
  example, to get the MD5 hashes of the entire Windows directory tree, run
  "md5sum -r C:\Windows".
