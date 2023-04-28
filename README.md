# hen_splitter
#### Unpack PS3HEN.BIN file into sections and pack back into new file

#### Usage: hen_splitter.exe /unpack|/pack PS3HEN.BIN /out|/in [directory]

#### Drag and Drop is Supported

> #### Commands:
> - /unpack               Unpack the PS3HEN.BIN file into separate sections
> - /out                  Specify output directory for PS3HEN.BIN
> - /pack                 Pack the section files back into a new file
> - /in                   Specify input directory for sections<br><br>
> - Unpack Example 1:     hen_splitter.exe /unpack PS3HEN.BIN
> - Unpack Example 2:     hen_splitter.exe /unpack PS3HEN.BIN /out "490C"
> - Pack Example 1:       hen_splitter.exe /pack PS3HEN_NEW.BIN
> - Pack Example 2:       hen_splitter.exe /pack PS3HEN_NEW.BIN /in "490C"