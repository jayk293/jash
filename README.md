A simple shell that abstracts a provided command and executes it with every argument given.

Example usage with Docker
```
./jash docker
```

The shell will look like this: 

```
jash <currentfolder>: docker
```

Docker will now be executed with every command, so you dont have to worry about typing `docker images`, 
instead you only need to type `images` and it will execute as if you typed `docker images`

If you need to execute a another command that is not the one you initialsed with, you can type it freely but prepend it with a `\`.
e.g.

```
jash <currentfolder>: docker \ls
```