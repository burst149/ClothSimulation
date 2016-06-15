#Output name
TARGET = Cloth.exe

#Project directories
INCDIR = inc
SRCDIR = src
OBJDIR = obj
BINDIR = bin

#Search directories
IDIR = -I./extlibs/glfw/include -I./extlibs/gl3w/include -I./extlibs/linmath.h
LDIR = -L./extlibs/glfw/lib -L./extlibs/gl3w/lib

#Project Files
SRC = $(call rwildcard, $(SRCDIR)/,*.cpp,)
INC = $(call rwildcard, $(INCDIR)/,*.hpp,)

#Flags
CC = g++
CFLAGS = -Wall -s -static -g
#-mwindows
LFLAGS = -lgl3w -lglfw3 -lopengl32 -lgdi32

#Commands
MKD = @if not exist $(subst $(SRCDIR),$(OBJDIR),$(subst /,\,$(@D))) mkdir $(subst $(SRCDIR),$(OBJDIR),$(subst /,\,$(@D)))
CMD = @echo Compiling: $@ && @$(CC) $(CFLAGS) -I./$(INCDIR) $(IDIR) -c $< -o $@

#Recursive wildcard
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

#Dependencies
DEPS = depend

#Done
all: $(BINDIR)/$(TARGET)
	@echo Done!

#Directories
$(BINDIR):
	@mkdir $(BINDIR)

#Linking
$(BINDIR)/$(TARGET): $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(SRC:.cpp=.obj)) | $(BINDIR)
	@echo Linking: $@
	@$(CC) $(CFLAGS) $^ $(LDIR) $(LFLAGS) -o $@

#Building dependencies
$(DEPS): $(SRC) $(INC)
	@echo.> $(DEPS) && $(foreach x,$(SRC),$(CC) -I./$(INCDIR) $(IDIR) -MM $(x) -MT $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(x:.cpp=.obj)) >> $(DEPS) && @echo 	$$(MKD) >> $(DEPS) && @echo 	$$(CMD) >> $(DEPS) && ) @echo.>> $(DEPS)

#Compiling
-include $(DEPS)


