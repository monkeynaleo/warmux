CXX=g++
CXXFLAGS:=-g -DDEBUG -Wall -Werror $(CXXFLAGS)

OBJ=basic_config.o basic_clock.o

common.a: $(OBJ)
	ar rcs wormux_server_common.a $(OBJ)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -c -o $@ $<

clean:
	@rm -f *.[oda] *~ 2>&1 > /dev/null

-include $(OBJ:.o=.d)
