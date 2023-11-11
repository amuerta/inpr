/*
 MIT License
  Copyright (c) 2023 amurtasmail@gmail.com

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and 
    associated documentation files (the "Software"), to deal in the Software without restriction, 
    including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial
    portions of the Software.
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
    LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH 
    THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
*/

// What program should do:
// + read file of any ini-like format
// + get varibles values and print them to STDOUT
// + change varible values 
// + be able to read categories and search within thier scope
// + on fail print to STDERR
// + handle __most__ edgecases and errors

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define IS_OK 1

#define DEFAULT_INDENT 1

#define ERROR -1
#define EMPTY  0
#define FULL   1+
#define IDNT   0
#define SET    2
#define READ   6
#define SECT   8
#define EQCH   4
#define HELP   10


char* ARGS[] = 
{
  "-i",  "--indent",
  "-s",  "--set",
  "-c",  "--character",
  "-r",  "--read",
  "-S",  "--section",
  "-h",  "--help"
};



int     OK              (int expr);

FILE*   Init            (char** args);
void    Set             (FILE* f,char** argv,uint argc,uint set_pos,char ARG_CHAR,uint indent,int OVERRIDE_POS);
void    Read            (FILE* f,char** argv,uint argc,uint read_pos,char ARG_CHAR,int OVERRIDE_POS);
int     getChar         (char** argv,uint argc,uint pos);
int     getIndent       (char** argv,uint argc,uint pos);
int     checkSection    (FILE *f, char** argv, uint argc, uint arg_pos, char* var);

void    swap            (void* item1,void* item2,size_t len);
char*   strTrim         (char* str, char* chars_to_trim);
void    strMerge        (char* target, char* src);
void    die             (const char *error, FILE* fclos);
FILE*   openFile        (char* path);
void    jumpToLine      (FILE *f, uint lines_to_skip);
uint    linesInFile     (FILE *f);
int     findPatternLine (FILE *f, char* pat);
int     findPatternSkip (FILE *f, char* pat,int lines_to_skip);
uint    findHeaderLine  (FILE *f, char* name);
uint    sizeHeader      (FILE *f, uint header_position);
char*   readValue       (FILE *f, uint line);
char*   readValueSignd  (FILE *f, uint line,char assignment_char);
char*   copyRange       (FILE *f, uint being, uint end);
int     setValue        (FILE *f, uint line,char* value);
int     setValueSignd   (FILE *f, uint line,char eq_char,char* value,uint indent);





int main 
  (int argc, char **argv)
{
  if(argc >= 2)
  {
    enum Operation { O_Read,O_Write } Operation;

    int 
      INDENT = DEFAULT_INDENT,
      SECTION_VAR = -1;
    ;

    int
      ARG_CHAR = -1,
      act_pos = -1,
      eqch_pos = -1,
      indt_pos = -1,
      sect_pos = -1
    ;

    FILE *f; //= openFile(arg);

    for (uint i = 0; i < argc; i++)
    {
      
     if 
        (
         strcmp(argv[i], ARGS[READ]) == 0 ||
         strcmp(argv[i], ARGS[FULL READ]) == 0 
        ) 
       {
         Operation = O_Read;
         act_pos = i;
       }
     else if 
       (
         strcmp(argv[i], ARGS[EQCH]) == 0 ||
         strcmp(argv[i], ARGS[FULL EQCH]) == 0 
       ) 
      {
        eqch_pos = i;
      }
     else if 
       (
         strcmp(argv[i], ARGS[SET]) == 0 ||
         strcmp(argv[i], ARGS[FULL SET]) == 0 
       ) 
      {
        Operation = O_Write;
        act_pos = i;
      }
     else if 
       (
         strcmp(argv[i], ARGS[IDNT]) == 0 ||
         strcmp(argv[i], ARGS[FULL IDNT]) == 0 
       ) 
     {
       indt_pos = i;
     }
     else if 
        (
         strcmp(argv[i], ARGS[SECT]) == 0 ||
         strcmp(argv[i], ARGS[FULL SECT]) == 0 
        )
      {
        sect_pos = i;
      }
    }


    if(OK(eqch_pos))
      ARG_CHAR = getChar(argv, argc, eqch_pos);

    if(OK(indt_pos))
      // printf("indent -> %d",INDENT);
      INDENT = getIndent(argv, argc, indt_pos);


    if(OK(act_pos))
    {
      f = Init(argv);

      if (OK(sect_pos)) 
      {
        SECTION_VAR = checkSection(f, argv, argc, sect_pos,argv[act_pos+1]);
        if (!OK(SECTION_VAR)) die("Err: No section name provided, or no varible found",f);
      }

      if (Operation == O_Read)
        Read(f, argv, argc, act_pos, ARG_CHAR,SECTION_VAR);
      else
        Set(f, argv, argc, act_pos, ARG_CHAR,INDENT,SECTION_VAR);
    }
   
    
  }
  else
  {
    printf
      (
       "%s%s%s\n%s\n%s\n%s\n%s\n\n%s",
       "USAGE:\n",
       " inpr <FILE> [FLAGS] [READ/WRITE OPERATION] \n\n",
       " [ -s | --set       ] <VAR> <VALUE>      - set variable to value",
       " [ -r | --read      ] <VAR>              - print variable value to STDOUT",
       " [ -c | --character ] <CHAR> <OPERATION> - character after which you want to read value",
       " [ -i | --indent    ] <INT>              - indent varible from equality_character",
       " [ -S | --section   ] <STR>              - section in which you wanna find the varible",
       "\n"
       "EXAMPLES:\n"
       " inpr config.ini -c = -r \"server\" \n" 
       "     (?) will search value of server varible in config.ini\n\n" 
       " inpr keys.ini -S \"Admin\" -c = -r \"passcode\"\n"
       "     (?) will search in keys.ini file for varible passcode in section called Admin\n\n"
       " inpr dock.ini --indent 1 --Section \"misc\" --character = --set \"clock\" \"false\" \n"
       "     (?) will set clock to false in misc section of dock.ini file\n\n"
       " inpr clock.yaml -indent 1 --character : --set \"detailed\" \"true\" \n"
       "     (?) will set detailed to true in .yaml file\n\n\n"
       " NOTE : Section search not gonna work unless the format is followed : \n\n\t[<ANY_NAME_WITHOUT_SPACES_OR_UTF8>]"
      );
  }
}



int   OK
  (int expr)
{
  return (expr==-1)? false : true;
}


FILE*   Init            
(char** args) 
{
  char* file_name;
  FILE* f;
  file_name = (char*)malloc(sizeof(char)*strlen(args[1])); // this caused so much headache
  file_name = args[1];
  f = openFile(file_name);
  if (f == 0) die("Err: failed to open file",0);
  return f;
}


void  Set             
  (FILE* f,char** argv,uint argc,uint set_pos,char ARG_CHAR,uint indent,int OVERRIDE_POS)
{
  int var = set_pos + 1;
  int val = set_pos + 2;
  int line;




  if (argc >= set_pos+2)
  {
    line = (OK(OVERRIDE_POS))? OVERRIDE_POS : findPatternLine(f, argv[var]);
    if (line!=ERROR)
    {
      if (ARG_CHAR!=ERROR)
        setValueSignd(f, line, (char)ARG_CHAR, argv[val],indent);
      else
        setValue(f, line, argv[val]);
    }
    else 
      die("Err: not found variable or file specified wrong",0);
  }
  else 
    die("not enough arguments provided\n" 
        "[EXAMPLE]: $ inpr file.ini --set \"server\" \"127.0.0.1\"",  f);
}


void   Read            
  (FILE* f,char** argv,uint argc,uint read_pos,char ARG_CHAR,int OVERRIDE_POS)
{
  int line;

  if (argc >= read_pos+1)
  {
    line = (OK(OVERRIDE_POS))? OVERRIDE_POS : findPatternLine(f, argv[read_pos+1]);
    if (line!=ERROR)
    {
      char* out;

      if(ARG_CHAR!=ERROR)
        out = readValueSignd(f, line, (char)ARG_CHAR) ;
      else 
        out = readValue(f, line);
      fprintf(stdout, "%s" ,out);
    }
    else 
      die("Err: not found variable or file specified wrong",0);
  }
  else die("Err: no var to read where provided",f);
}



int   checkSection    
    (FILE *f, char** arg, uint argc, uint arg_pos,char* var)
{
  int pos = arg_pos;

  if (arg_pos!=ERROR)
  {
    if(argc >= arg_pos+1)
      pos = arg_pos+1;
    else 
    {
      die("Err: section name wasnt provided",0);
    }
    printf("\b"); // magic, i honestly have no idea why.
                  // but this particular print statment
                  // stops program from crashing.......
                  // Some day i will learn why;
    if (
        findHeaderLine(f, arg[pos]) != ERROR && 
        sizeHeader(f, findHeaderLine(f, arg[pos]))
       )
    {
      uint start = findHeaderLine(f, arg[pos])-1;
      uint end = start + sizeHeader(f, findHeaderLine(f,arg[pos]));
      for (uint i = start; i < end; i++)
      {
        if (findPatternSkip(f, var, start)) {
          int p = findPatternSkip(f, var,start);
          return start + p;
        }
      }
      die("Err: no varible within range found",f);
    }
  }
  return ERROR; // to ignore compiler warns
}


int   getChar       
    (char** argv,uint argc,uint pos)
{
  int eqch_pos = pos;

  if (eqch_pos!=ERROR)
  {
    char *echar = (char*) malloc(sizeof(char)*2);
    if (argc>=eqch_pos+1)
      strcpy(echar, argv[eqch_pos+1]);
    //echar argv[eqch_pos+1];
    else {
      free(echar);
      die("Err: Equality char wasnt provided",0);
    }
    if (strlen(echar)==1) {
      return echar[0];
    }
    else
    {
      die("Err: with -c | --character flag you provided <Str> type, not <Char> type",0);
    }
  }
  return ERROR;
}

int getIndent
    (char** argv,uint argc,uint pos)
{
  if (pos!=ERROR)
  {
    char *arg = (char*) malloc(sizeof(char)*2);
    if(argc >= pos+1)
      strcpy(arg, argv[pos+1]);
    else 
    {
      free(arg);
      die("Err: indent value wasnt provided",0);
    }

    return atoi(arg);
  }
  return DEFAULT_INDENT;
}

void swap(void * a, void * b, size_t len)
{
    unsigned char * p = a, * q = b, tmp;
    for (size_t i = 0; i != len; ++i)
    {
        tmp = p[i];
        p[i] = q[i];
        q[i] = tmp;
    }
}



char* strTrim         
  (char* str, char* chars_to_trim)
{
  char* retstr = (char*) malloc(sizeof(char)*2);
  bool trim_current_char = false;
  for(int i = 0; i < strlen(str); i++)
  {
    for(int sel = 0; sel < strlen(chars_to_trim); sel++)
    {
      trim_current_char = (chars_to_trim[sel]==str[i]);
      if (trim_current_char) break;
    }

    if (!trim_current_char)
    {
      char c[2] = {str[i],'\0'};
      strMerge(retstr, c);
    }
  }
  return retstr;
}


void  strMerge        
  (char* target, char* src)
{
  // damn
  strcat(target, src);
}



void die
  (const char *error, FILE* fclos)
{
  fprintf(stderr, "%s",error);
  if (fclos != 0) {fclose(fclos);}
  exit(-1);
}




FILE* openFile
  (char* path)
{
  FILE *f = fopen(path, "rw+");

  if (f == EMPTY) 
    die("Err: provided file path is incorrect",0);
  else 
    return f;
  return 0; // i dont like compiler warning msgs
}



void  jumpToLine
  (FILE *f, uint lines_to_skip)
{
  char ch;
  uint lines=0;

  rewind(f);


  for(;;)
  {
    if (lines_to_skip==0) break;

    ch = fgetc(f);
    if(ch==EOF) 
      break;
    else if (ch=='\n') {
      if (lines == lines_to_skip - 1)
        break;
      else
        lines++;
    }
  }
}



uint linesInFile
  (FILE *f)
{

  rewind(f);

  char ch;
  uint lines = 0;

  for(;;)
  {
    ch = fgetc(f);
    if(ch==EOF) 
      break;
    else if (ch=='\n')
      lines++;
  }
  return lines;
}



int findPatternLine
  (FILE *f,char* pat)
{
  char* word = (char *) malloc(sizeof(char)*2);//[128];
  char ch   = 0;
  uint lines= 0;
  bool read = true;

  rewind(f);

  for(;;)
  {
    ch = fgetc(f);

    if(ch == EOF) 
      break;
    else if (ch=='\n') {
      
      read = false;
      lines++;
    }
    else if (ch==' '|| ch=='\t')
      read = false;
    else 
      read = true;

    if (read)
    {
      char str[2];
      str[1] = '\0';
      str[0] = ch;
      
      strMerge(word, str);
    }
    else {
      if (strcmp(word,pat)==0) 
      {
        free(word);
        return lines;
      }
      else 
        memset(word, 0, strlen(word));
    }
  }
  free(word);
  return ERROR;
}



int   findPatternSkip 
  (FILE *f, char* pat,int lines_to_skip)
{

  char* word = (char *) malloc(1);//[128];
  char ch   = 0;
  uint lines= 0;
  bool read = true;

  strcpy(word,"");

  rewind(f);
  jumpToLine(f, lines_to_skip);

  for(;;)
  {
    ch = fgetc(f);

    if(ch == EOF) 
      break;
    else if (ch=='\n') {
      read = false;
      lines++;
    }
    else if (ch==' '|| ch=='\t')
      read = false;
    else 
      read = true;

    if (read)
    {
      char str[2];
      str[1] = '\0';
      str[0] = ch;
      sprintf(word, "%s%c",word, ch);
    }
    else {
      if (strcmp(word,pat)==0) 
      {
        free(word);
        return lines;
      }
      else
      {
        strcpy(word, "");
      }
    }
  }

  free(word);
  return ERROR;
}


uint findHeaderLine
  (FILE *f, char* name)
{
  char *str = (char*) malloc((3+strlen(name)) );
  sprintf(str, "[%s]", name);
  rewind(f);
  int line = findPatternLine(f, str);
  free(str);
  return line;
}



uint sizeHeader      
  (FILE *f, uint header_position)
{
  rewind(f);
  jumpToLine(f, header_position);


  uint size=0;
  char ch;
  bool open = false, close = false;

  for(;;)
  {
    ch = getc(f);

    if (ch == EOF)
      return size;
    else
    {
      if (ch == '[') open = true;
      else if (ch == ']') close = true;
      else if (ch == '\n') size++;

      if (open && close && size == 0 ) return EMPTY;
    }
    if (open && close) return size;
  }
}




char* readValue       
  (FILE *f, uint line)
{
  rewind(f);
  jumpToLine(f, line);

  char* val = malloc(2);
  char ch;
  bool
    read = false,
    whitespace = false
  ;
  uint words = 0;

  strcpy(val, "");

  for(;;)
  {
    ch = getc(f);
    if(ch!=' ' && ch!='\t')
      break;
  }

  for(;;)
  {
    ch = getc(f);

    if (ch == '\n' ||  ch == EOF)
      break;
    else if (ch!=' ' && whitespace)
    {
      words++;
      whitespace = false;
    }

    if (ch == ' ')
      whitespace = true;
    else
      read = true;
 
    if (read && words != 0)
    {
      sprintf(val,"%s%c",val,ch);
    }
  }
  return val;
}



// same as readValue, except
// we start reading only after assignment_char
// this meant for every file where syntax
// looks like this
//    var = value
//    var : value
//    var _ value, and etc
// we can get values to put in stdout
char*   readValueSignd  
  (FILE *f, uint line,char assignment_char)
{
  rewind(f);
  jumpToLine(f, line);

  bool 
    read=false
  ;
  char ch;
  char *val = malloc(sizeof(char)*2);

  strcpy(val, "");

  for(;;)
  {
    ch = getc(f);

    if (read && ch!=EOF)
    {
      sprintf(val, "%s%c", val , ch);
    }

    if (ch == EOF || ch == '\n')
      break;
    else if (ch == assignment_char)
      read = true;
  }
  return val;
}



char* copyRange       
  (FILE *f, uint line_begin, uint line_end)
{
  uint lcount = 0;
  char* str = (char*) malloc(sizeof(char)*10);

  rewind(f);
  if (line_begin > line_end)
    swap(&line_begin, &line_end,sizeof(uint));

  jumpToLine(f, line_begin);

  while(line_begin+ lcount != line_end)
  {
    char ch = getc(f);

    if (ch == '\n') 
    {
      str = realloc(str, strlen(str)+1);

      lcount++;
      strMerge(str, "\n");
    }
    else 
    {

      str = realloc(str, strlen(str)+2);

      char c[2];
      c[0] = ch;
      c[1] = '\0';
      strMerge(str, c);
    }
  }
  return str;
}




int  setValue 
  (FILE *f,uint line,char* value)
{
  char 
    *before, 
    *after,
    *token,
    *newtoken = malloc(sizeof(char)),
    *file = malloc(sizeof(char))
  ;

  rewind(f);

  strcpy(newtoken, ""); // to clean garbage

  before = copyRange(f, 0, line);
  after = copyRange(f, line+1, linesInFile(f));

  token = copyRange(f, line, line+1);

  bool last_read = false;
  bool read = false;
  uint words = 0;


  for(uint i = 0; i < strlen(token);i++)
  {
    char ch = token[i];
    last_read = read;
    
    if (!read && (ch!=' ' && ch!='\t' && ch != '\n') )
    {
      read = true;
    }
    if (read && !(ch!=' ' && ch!='\t' && ch != '\n') )
    {
      read = false;
    }

    if (last_read && !read) 
      words++;

    // token claiming
    if(words < 1)
    {
      char c[2] = {ch,'\0'};
      strMerge(newtoken, c );
    }

  }

  sprintf(newtoken, "%s %s", newtoken, value);
  sprintf(file, "%s%s\n%s",before,newtoken,after);
  
  rewind(f);

  fprintf(f,"%s","");
  fprintf(f,"%s",file);
  free(file);
  return 1;
}



int setValueSignd
  (FILE *f,uint line,char eq_char,char* value,uint indent)
{
  char 
    *before, 
    *after,
    *token,
    *newtoken = malloc(sizeof(char)),
    *ws = malloc(sizeof(char)),
    *file = malloc(sizeof(char))
      ;

  rewind(f);

  strcpy(newtoken, "");

  before = copyRange(f, 0, line);
  after = copyRange(f, line+1, linesInFile(f));

  token = copyRange(f, line, line+1);

  bool read = true;

  for(uint i = 0; i < strlen(token);i++)
  {
    char ch = token[i];
    if(ch == EOF)
      break;
    else if (ch == eq_char)
      read = false;
    else
      if (read) 
      {
        char c[2] = {ch,'\0'};
        strMerge(newtoken, c);
      }
  }



  for (uint i = 0; i < indent; i++)
    strMerge(ws, " ");

  sprintf(newtoken, "%s%c%s%s",newtoken,eq_char, ws,value);
  sprintf(file, "%s%s\n%s",before,newtoken,after);

  rewind(f);

  fprintf(f,"%s","");
  fprintf(f,"%s",file);

  free(file);
  return 1;
}

