# Unified C++ Makefile
# Author: k1-801

RESULT    = Life

SRCDIR    = src
OBJDIR    = obj
BINDIR    = bin
# Please rename or remove old backup directory if changed to avoid backuping old backups
BACKUPDIR = backups

LIBS      = -lm -lSDL2 -lSDL2_image -lSDL2_ttf
IS        = -I$(SRCDIR) $(patsubst %,-I%,$(shell find -type d -path "$(SRCDIR)/*"))
DEFINES   =
FLAGS     = -O2 -Wall $(DEFINES) $(IS) $(TARFLAGS)

# Change if needed
COMPILER  = g++

DATE      = $(shell date +%Y_%m_%d_%H_%M_%S)
ARCH      = $(shell uname -s)_$(shell uname -m)
SOURCES   = $(shell find -iname *.cpp)
OBJECTS   = $(patsubst ./$(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

GCC_USE_COLORS=always

vpath %.cpp $(SRCDIR)
vpath %.o   $(OBJDIR)

# Meta-targets
all:     build
build:   -p1 $(OBJECTS) link -p3 tar

Debug:   PREFPATH = bin/Debug/
Debug:   TARFLAGS = -g
Debug:   build

Release: PREFPATH = bin/Release
Release: TARFLAGS = -s
Release: build

# MEEEEEEEAAAT
help:
	@echo
	@echo "Usage:"
	@echo " make help       print this help"
	@echo " make clean      remove redundant data"
	@echo
	@echo " make build      force rebuild"
	@echo " make all        [build]"
	@echo
	@echo " make Debug      rebuild with debug symbols"
	@echo " make Release    rebuild with stripped symbols"
	@echo
	@echo " make [object]   compile [object]"
	@echo
	@echo " Objects to be compiled: $(OBJECTS)"
	@echo
	@echo " make link       link project"
	@echo
	@echo "$(ARCH)"
	@echo

# Just pre-dependenced output
-p1:
	@echo " => Build [$(RESULT)]: started for $(ARCH) at $(DATE)"
-p2:
	@echo " => Compiling"
-p3:
	@echo " => Build [$(RESULT)]: success"

# Real working
clean:
	@echo " => Cleaning"
	@rm -f `find -iname "*.o"`

$(OBJDIR)/%.o: -p2
	@echo " => Compiling [$@]"
	@mkdir -p ./$(dir $@)
	@$(COMPILER) $(FLAGS) -c $(patsubst $(OBJDIR)/%.o,$(SRCDIR)/%.cpp,$@) -o $@

link:
	@echo " => Linking [$(RESULT)]"
	@mkdir -p ./$(PREFPATH)
	@$(COMPILER) $(LIBS) $(OBJECTS) -o ./$(PREFPATH)$(RESULT)
	@cp ./$(PREFPATH)$(RESULT) ./$(RESULT)_$(ARCH)
	@echo " => Output file is ./$(PREFPATH)$(RESULT)"

tar:
	@echo " => Creating backup"
	@mkdir -p $(BACKUPDIR)
	@tar -cf $(BACKUPDIR)/$(RESULT)_$(DATE).tar $(filter-out $(BACKUPDIR),$(shell ls))

noreturn:
	@echo " => Removing ALL backups"
	@echo " => I hope you know, what you do, and your backup directory path is set"
	@echo " => Removing [$(BACKUPDIR)]"
	@rm -Rf ./$(BACKUPDIR)
	@echo " => Backups were removed successfully"
	@echo " => No return anymore"
