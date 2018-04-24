% column vectors to matrix organization for central_grid.roi

nx = 90;
ny = 52;

xm = zeros(ny,nx);
ym = zeros(ny,nx);
zm = zeros(ny,nx);

um = zeros(ny,nx);
vm = zeros(ny,nx);
wm = zeros(ny,nx);

for i=1:nx
    for j=1:ny
        indx = (i-1)*ny + j;
        
        xm(j,i) = x(indx);
        ym(j,i) = y(indx);
        zm(j,i) = x(indx);
        
        um(j,i) = u(indx);
        vm(j,i) = v(indx);
        wm(j,i) = w(indx);
    end
end




