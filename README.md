# Introduction



# Directory Structure

* buildsystem
* fast-dds-iceoryx-allin
* non-stardard-com
* rpc_method
* vrte
* vsomeip-come-qnx
* vsomeip-qnx-allin

# large file support

Because shared library files generated from vsompip is too large ( > 100MB ), git-lfs should be installed.

```sudo apt install git-lfs```

add large file into track:

```git lfs track <path_to_filename>```

.gitattributes already be added into git, make sure .gitattributes file be commited if new large file added.

