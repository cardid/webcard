#if defined(_WIN32)
  #error("WIN32 not supported yet!")
  #pragma GCC error "WIN32 not supported yet!"

  #include <windows.h>

  #define WEBCARD_EXEC  "webcard.exe"

#elif defined(__linux__) || defined(__APPLE__)

  #include <stdint.h>  /* uint32_t */
  #include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
  #include <stdio.h>  /* perror */
  #include <string.h>  /* strlen */
  #include <unistd.h>  /* execl, fork, fork, write, read, close, STDIN_FILENO, STDOUT_FILENO, */
  #include <fcntl.h>  /* O_NONBLOCK */
  #include <errno.h>

  typedef int BOOL;
  #define FALSE  0
  #define TRUE   1

  #define WEBCARD_EXEC  "webcard"

  #define READ_END   0
  #define WRITE_END  1

#else
  #error("Unsupported Operating System, sorry!")
  #pragma GCC error "Unsupported Operating System, sorry!"
#endif

/**************************************************************/

#define BUF_LENGTH 256

#define EXAMPLE_JSON  "{\"i\":\"WEBCARD_TEST\",\"c\":1}"

/**************************************************************/

char
nibble_to_char(const uint8_t code)
{
  if (code < 10)
  {
    return ('0' + code);
  }
  else if (code < 16)
  {
    return (code - 10 + 'a');
  }

  return '?';
}

/**************************************************************/

void
write_debug_text(const char *text)
{
  write(STDERR_FILENO, text, strlen(text));
}

/**************************************************************/

void
write_debug_utf8(const char *text, uint32_t length)
{
  char escaped[4] = {'\\'};

  for (size_t i = 0; i < length; i++)
  {
    uint8_t code = text[i];

    if ((code < ' ') || (0x80 & code))
    {
      escaped[1] = 'x';
      escaped[2] = nibble_to_char(0x0F & (code >> 4));
      escaped[3] = nibble_to_char(0x0F & code);
      write(STDERR_FILENO, escaped, 4);
    }
    else if (('\\' == code) || ('"' == code))
    {
      escaped[1] = code;
      write(STDERR_FILENO, escaped, 2);
    }
    else
    {
      write(STDERR_FILENO, &(code), 1);
    }
  }
}

/**************************************************************/

BOOL
pipe_send(const char *name, int fd_write, const char *utf8_text, uint32_t length)
{
  write_debug_text(" @ ");
  write_debug_text(name);
  write_debug_text(" send start: ");
  write_debug_utf8(utf8_text, length);
  write_debug_text("\n");

  if (sizeof(uint32_t) != write(fd_write, &(length), sizeof(uint32_t)))
  {
    perror(" @ write(fd_write, &(length), sizeof(uint32_t))");
    return FALSE;
  }

  if (length != write(fd_write, utf8_text, length))
  {
    perror(" @ write(fd_write, utf8_text, length)");
    return FALSE;
  }

  write_debug_text(" @ ");
  write_debug_text(name);
  write_debug_text(" send end.\n");

  return TRUE;
}

/**************************************************************/

BOOL
pipe_recv(const char *name, int fd_read, char utf8_buf[BUF_LENGTH])
{
  uint32_t length;
  BOOL repeat;

  write_debug_text(" @ ");
  write_debug_text(name);
  write_debug_text(" recv start.\n");

  repeat = TRUE;
  while (repeat)
  {
    if (sizeof(uint32_t) != read(fd_read, &(length), sizeof(uint32_t)))
    {
      if (EAGAIN != errno)
      {
        perror(" @ read(fd_read, &(length), sizeof(uint32_t))");
        return FALSE;
      }
    }
    else
    {
      repeat = FALSE;
    }
  }

  if (length > BUF_LENGTH)
  {
    write_debug_text(" @ (length > BUF_LENGTH)");
    return FALSE;
  }

  repeat = TRUE;
  while (repeat)
  {
    if (length != read(fd_read, utf8_buf, length))
    {
      if (EAGAIN != errno)
      {
        perror(" @ read(fd_read, buf, length)");
        return FALSE;
      }
    }
    else
    {
      repeat = FALSE;
    }
  }

  write_debug_text(" @ ");
  write_debug_text(name);
  write_debug_text(" recv end: ");
  write_debug_utf8(utf8_buf, length);
  write_debug_text("\n");

  return TRUE;
}

/**************************************************************/

BOOL
parent_do_stuff(int fd_read, int fd_write)
{
  char buf[BUF_LENGTH];

  const size_t repeats = 10;

  write_debug_text("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

  /* Query card readers */

  if (!pipe_send("Parent", fd_write, EXAMPLE_JSON, strlen(EXAMPLE_JSON)))
  {
    return FALSE;
  }

  if (!pipe_recv("Parent", fd_read, buf))
  {
    return FALSE;
  }

  for (size_t i = 0; i < repeats; i++)
  {
    snprintf(buf, BUF_LENGTH, " @ (%zu/%zu)\n", (1 + i), repeats);

    write_debug_text("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    write_debug_text(buf);

    /* Get one card event */

    if (!pipe_recv("Parent", fd_read, buf))
    {
      return FALSE;
    }

    /* Query card readers */

    if (!pipe_send("Parent", fd_write, EXAMPLE_JSON, strlen(EXAMPLE_JSON)))
    {
      return FALSE;
    }

    if (!pipe_recv("Parent", fd_read, buf))
    {
      return FALSE;
    }
  }

  write_debug_text("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

  return TRUE;
}

/**************************************************************/

void
prepare_child_path(int argc, char ** argv, char result[BUF_LENGTH])
{
  strncpy(
    &(result[0]),
    (argc >= 2)
      ? &(argv[1][0])
      : WEBCARD_EXEC,
    BUF_LENGTH);

  result[BUF_LENGTH - 1] = '\0';
}

/**************************************************************/

int
main(int argc, char ** argv)
{
  char buf[BUF_LENGTH];
  int pipe_child_to_parent[2];
  int pipe_parent_to_child[2];
  pid_t child_pid;
  int result;

  if ((-1) == pipe(pipe_child_to_parent))
  {
    perror("pipe(pipe_child_to_parent)");

    return EXIT_FAILURE;
  }

  if ((-1) == pipe(pipe_parent_to_child))
  {
    perror("pipe(pipe_parent_to_child)");

    close(pipe_child_to_parent[READ_END]);
    close(pipe_child_to_parent[WRITE_END]);
    return EXIT_FAILURE;
  }

  if ((-1) == (child_pid = fork()))
  {
    perror("fork()");

    close(pipe_child_to_parent[READ_END]);
    close(pipe_child_to_parent[WRITE_END]);
    close(pipe_parent_to_child[READ_END]);
    close(pipe_parent_to_child[WRITE_END]);
    return EXIT_FAILURE;
  }

  if (0 == child_pid)
  {
    /* Child process */

    close(pipe_child_to_parent[READ_END]);
    close(pipe_parent_to_child[WRITE_END]);

    write_debug_text(" @ Hello from the child process!\n");

    result = dup2(pipe_parent_to_child[READ_END], STDIN_FILENO);
    close(pipe_parent_to_child[READ_END]);

    if ((-1) == result)
    {
      perror(" @ dup2(pipe_parent_to_child[READ_END], STDIN_FILENO)");

      close(pipe_child_to_parent[WRITE_END]);
      return EXIT_FAILURE;
    }

    result = dup2(pipe_child_to_parent[WRITE_END], STDOUT_FILENO);
    close(pipe_child_to_parent[WRITE_END]);

    if ((-1) == result)
    {
      perror(" @ dup2(pipe_child_to_parent[WRITE_END], STDOUT_FILENO)");

      return EXIT_FAILURE;
    }

    /* Set proper executable path name */
    prepare_child_path(argc, argv, buf);

    write_debug_text(" @ Child process will try to launch: \"");
    write_debug_text(buf);
    write_debug_text("\"\n");

    /* Change process to "Webcard Native App" */
    if ((-1) == execl(buf, buf, NULL))
    {
      perror(" @ execl()");
      return EXIT_FAILURE;
    }
  }
  else
  {
    /* Parent process */

    close(pipe_child_to_parent[WRITE_END]);
    close(pipe_parent_to_child[READ_END]);

    sleep(1);
    write_debug_text(" @ Hello from the parent process!\n");

    if (0 != fcntl(pipe_child_to_parent[READ_END], F_SETFL, O_NONBLOCK))
    {
      perror("fcntl(pipe_child_to_parent[READ_END], F_SETFL, O_NONBLOCK)");
    }
    else
    {
      parent_do_stuff(
        pipe_child_to_parent[READ_END],
        pipe_parent_to_child[WRITE_END]);
    }

    close(pipe_child_to_parent[READ_END]);
    close(pipe_parent_to_child[WRITE_END]);

    write_debug_text(" @ Parent process says goodbye...\n");
  }

  return EXIT_SUCCESS;
}

/**************************************************************/
