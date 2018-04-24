

inc = include/
obj = include/objects/
eig = $(inc)eigen

objects = 	$(obj)Point.o \
		$(obj)BoundBox.o \
		$(obj)Cloud.o \
		$(obj)DataCloud.o \
		$(obj)FloatingCloud.o \
		$(obj)Matrix_4d.o \
		$(obj)Interpolate.o \
		$(obj)ObjectiveFunctions.o \
		$(obj)SearchParams.o \
		$(obj)Search.o \
		$(obj)InputRead.o \
		$(obj)dvc.o

		
dvc: $(objects)
	g++ -o $@ $(objects)

	
$(obj)Point.o: $(inc)Point.cpp $(inc)Point.h
			g++ -O3 -c -o $@ $<

$(obj)BoundBox.o: $(inc)BoundBox.cpp $(inc)BoundBox.h $(inc)Point.h $(inc)Utility.h
			g++ -O3 -c -o $@ $<
			
$(obj)InputRead.o: $(inc)InputRead.cpp $(inc)InputRead.h $(inc)Point.h $(inc)BoundBox.h $(inc)Utility.h
			g++ -O3 -c -o $@ $<
						
$(obj)Cloud.o: $(inc)Cloud.cpp $(inc)Cloud.h $(inc)Point.h $(inc)BoundBox.h 
			g++ -O3 -c -o $@ $<
			
$(obj)DataCloud.o: $(inc)DataCloud.cpp $(inc)DataCloud.h $(inc)Point.h $(inc)Cloud.h $(inc)InputRead.h $(inc)Utility.h
			g++ -O3 -c -o $@ $<
					
$(obj)FloatingCloud.o: $(inc)FloatingCloud.cpp $(inc)FloatingCloud.h $(inc)Point.h $(inc)Cloud.h $(inc)SearchParams.h
			g++ -O3 -c -o $@ $<
			
$(obj)Matrix_4d.o: $(inc)Matrix_4d.cpp $(inc)Matrix_4d.h
			g++ -O3 -I $(eig) -c -o $@ $<
						
$(obj)Interpolate.o: $(inc)Interpolate.cpp $(inc)Interpolate.h $(inc)Point.h $(inc)BoundBox.h $(inc)Matrix_4d.h $(inc)Utility.h
			g++ -O3 -I $(eig) -c -o $@ $<

$(obj)SearchParams.o: $(inc)SearchParams.cpp $(inc)SearchParams.h
			g++ -O3 -c -o $@ $<
			
$(obj)ObjectiveFunctions.o: $(inc)ObjectiveFunctions.cpp $(inc)ObjectiveFunctions.h
			g++ -O3 -c -o $@ $<
		
$(obj)Search.o: $(inc)Search.cpp $(inc)Search.h $(inc)Point.h $(inc)BoundBox.h $(inc)InputRead.h $(inc)FloatingCloud.h $(inc)DataCloud.h $(inc)InputRead.h $(inc)ObjectiveFunctions.h $(inc)Utility.h
			g++ -O3 -I $(eig) -c -o $@ $<
			
			
$(obj)dvc.o: dvc.cpp $(inc)dvc.h $(inc)InputRead.h $(inc)DataCloud.h $(inc)Search.h $(inc)Utility.h 
			g++ -O3 -I include -I $(eig) -c -o $@ $<


clean: 
	rm dvc $(obj)*.o


