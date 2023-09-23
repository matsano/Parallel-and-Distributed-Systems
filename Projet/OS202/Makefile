# include à modifier selon sa configuration
#include make_msys.inc 
include make_linux.inc


ALL= vortexSimulation_q1_q2.exe vortexSimulation_q3.exe

default:	help
all: $(ALL)

clean:
	@rm -fr objs/*.o *.exe src/*~ *.png

OBJS12= objs/vortex.o objs/screen.o objs/runge_kutta.o objs/cloud_of_points.o objs/cartesian_grid_of_speed.o \
      objs/vortexSimulation_q1_q2.o

OBJS3= objs/vortex.o objs/screen.o objs/runge_kutta.o objs/cloud_of_points.o objs/cartesian_grid_of_speed.o \
      objs/vortexSimulation_q3.o

objs/vortex.o:	src/point.hpp src/vector.hpp src/vortex.hpp src/vortex.cpp
	$(CXX) $(CXXFLAGS) -Isrc -c -o $@ src/vortex.cpp

objs/cartesian_grid_of_speed.o: src/point.hpp src/vector.hpp src/vortex.hpp src/cartesian_grid_of_speed.hpp src/cartesian_grid_of_speed.cpp
	$(CXX) $(CXXFLAGS) -Isrc -c -o $@ src/cartesian_grid_of_speed.cpp

objs/cloud_of_points.o: src/point.hpp src/rectangle.hpp src/cloud_of_points.hpp src/cloud_of_points.cpp 
	$(CXX) $(CXXFLAGS) -Isrc -c -o $@ src/cloud_of_points.cpp 

objs/runge_kutta.o:	src/vortex.hpp src/cloud_of_points.hpp src/cartesian_grid_of_speed.hpp src/runge_kutta.hpp src/runge_kutta.cpp 
	$(CXX) $(CXXFLAGS) -Isrc -c -o $@ src/runge_kutta.cpp

objs/screen.o:	src/vortex.hpp src/cloud_of_points.hpp src/cartesian_grid_of_speed.hpp src/screen.hpp src/screen.cpp
	$(CXX) $(CXXFLAGS) -Isrc -c -o $@ src/screen.cpp

objs/vortexSimulation_q1_q2.o: src/cartesian_grid_of_speed.hpp src/vortex.hpp src/cloud_of_points.hpp src/runge_kutta.hpp src/screen.hpp src/vortexSimulation_q1_q2.cpp
#	$(CXX) $(CXXFLAGS) -Isrc -c -o $@ src/vortexSimulation_q1_q2.cpp
	$(MPICXX) $(CXXFLAGS) -Isrc -c -o $@ src/vortexSimulation_q1_q2.cpp

objs/vortexSimulation_q3.o: src/cartesian_grid_of_speed.hpp src/vortex.hpp src/cloud_of_points.hpp src/runge_kutta.hpp src/screen.hpp src/vortexSimulation_q3.cpp
#	$(CXX) $(CXXFLAGS) -Isrc -c -o $@ src/vortexSimulation_q3.cpp
	$(MPICXX) $(CXXFLAGS) -Isrc -c -o $@ src/vortexSimulation_q3.cpp

vortexSimulation_q1_q2.exe: $(OBJS12)
#	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LIB)
	$(MPICXX) $(CXXFLAGS) -o $@ $(OBJS12) $(LIB)

vortexSimulation_q3.exe: $(OBJS3)
#	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LIB)
	$(MPICXX) $(CXXFLAGS) -o $@ $(OBJS3) $(LIB)

help:
	@echo "Available targets : "
	@echo "    all                           : compile all executables"
	@echo "    vortexSimulation_q1.exe          : compile simple this executable"
	@echo "Add DEBUG=yes to compile in debug"
	@echo "Configuration :"
	@echo "    CXX      :    $(CXX)"
	@echo "    CXXFLAGS :    $(CXXFLAGS)"
