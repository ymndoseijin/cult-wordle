this is an old wordle clone I made using C that uses words files, I got it to compile to DOS as well, it's in the releases page. You need to supply it with a words file

it also supports utf-8

```
usage: ./cult-wordle [option] ...
       ./cult-wordle [option] ... dictionary-file

  -h, --help shows this help
  -v, --version outputs the "version"
  -o, --obscure obscure final score letters
  -c, --cheter CHETER!!
  -n, --fixed-num [length] fixes word length in game, by default, off
  -r, --random set seed to random mode, default daily seed
  -s, --seed set [seed] seed to [seed], default daily seed
```