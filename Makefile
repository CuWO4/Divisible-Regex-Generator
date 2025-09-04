TARGET := gen.exe
EXTERN := cpp
COMPILER := clang++

SRCDIR := src
INCLUDEDIR := include
# store .o and .d files #
TMPDIR := tmp
# store the target file #
DEBUGDIR := debug

COMPILE_OPTION := -Wall -O2 -I$(INCLUDEDIR)
# to generate dependent files #
COMPILE_OPTION_DES := -MMD -MP

RUNARGS ?=

# sources, objects and dependencies #
SRCS := $(wildcard $(SRCDIR)/*.$(EXTERN))
OBJS := $(patsubst $(SRCDIR)/%.$(EXTERN), $(TMPDIR)/%.o, $(SRCS))
DEPS := $(patsubst $(SRCDIR)/%.$(EXTERN), $(TMPDIR)/%.d, $(SRCS))

# link #
$(DEBUGDIR)/$(TARGET) : $(OBJS) | $(DEBUGDIR)
	@ echo linking...
	$(COMPILER) $(OBJS) -o $(DEBUGDIR)/$(TARGET)
	@ echo completed!

# compile #
$(TMPDIR)/%.o : $(SRCDIR)/%.$(EXTERN) | $(TMPDIR)
	@ echo compiling $<...
	$(COMPILER) $< -o $@ -c $(COMPILE_OPTION) $(COMPILE_OPTION_DES)

# create TMPDIR when it does not exist #
$(TMPDIR) :
	@ mkdir $(patsubst ./%, %, $(TMPDIR))

$(DEBUGDIR) :
	@ mkdir $(DEBUGDIR)

# files dependecies #
-include $(DEPS)

# run command #
.PHONY : run
run : $(DEBUGDIR)/$(TARGET)

	./$(DEBUGDIR)/$(TARGET) $(RUNARGS)

# clean command #
.PHONY : clean
clean :
	@ echo try to clean...
	rm -r $(DEBUGDIR)/$(TARGET) $(OBJS) $(DEPS)
	@ echo completed!