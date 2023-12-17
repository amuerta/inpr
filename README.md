# Inpr - ini parser (or processor)

INi PRocessor - is simple C program to _read_ and _write_ to `.INI`-like files. Read statment outputs values to `STDOUT`, Write sets variables to thier new provided values.

## DESCRIPTION:
```
USAGE:
 inpr <FILE> [FLAGS] [READ/WRITE OPERATION] 

 [ -s | --set       ] <VAR> <VALUE>      - set variable to value
 [ -r | --read      ] <VAR>              - print variable value to STDOUT
 [ -c | --character ] <CHAR> <OPERATION> - character after which you want to read value
 [ -i | --indent    ] <INT>              - indent varible from equality_character
 [ -S | --section   ] <STR>              - section in which you wanna find the varible


EXAMPLES:
 inpr config.ini -c = -r "server" 
     (?) will search value of server varible in config.ini

 inpr keys.ini -S "Admin" -c = -r "passcode"
     (?) will search in keys.ini file for varible passcode in section called Admin

 inpr dock.ini --indent 1 --Section "misc" --character = --set "clock" "false" 
     (?) will set clock to false in misc section of dock.ini file

 inpr clock.yaml -indent 1 --character : --set "detailed" "true" 
     (?) will set detailed to true in .yaml file

 NOTE : Section search not gonna work unless the format is followed : 

	[<ANY_NAME_WITHOUT_SPACES_OR_UTF8>] 

```

## How to install

__MAKE SHURE YOU HAVE__ `CLANG` __COMPILER INSTALLED__

1. Clone this repo
```
  $ git clone <repo-url>
```
2. cd into repo
```
  $ cd ./inpr/
```
3. make build,sh executable and run it.
```
  $ chmod +x ./build.sh
  $ ./build.sh
```

4. If you want this program to be usable from any location, move it to `/usr/bin`
```
  $ sudo mv inpr /use/bin/
```

## CONTRIBUTION
I am not very experienced developer, and have my steep learning curve with writing actual software, considering this is my first actually finished project, i am a noob, so if it happens that..

You know:
 
 - how to improve this program functions
 - make it faster
 - fix any error(s) you found
 - help make program source code and binary smaller
 
I will be __very__ thankful and happy for your help :)

## ALSO
If you really open for discussion and know how C memory managment is done..
I really want to know why program crashes with this error
```
main: malloc.c:2617: sysmalloc: Assertion `(old_top == initial_top (av) && old_size == 0) || ((unsigned long) (old_size) >= MINSIZE && prev_inuse (old_top) && ((unsigned long) old_end & (pagesize - 1)) == 0)' failed.
./build.sh: line 8: 73726 Aborted                 ./$NAME ${@:2}

```
when i remove `printf()` statment from line 303 in `checkSection()` function


Its something to do with memory corruption caused by unproper ``char *`` data manipulation, but i want to know ___exactly___ what is going on there.ts something to do with memory corruption caused by unproper ``char *`` data manipulation, but i want to know ___exactly___ what is going on there.
