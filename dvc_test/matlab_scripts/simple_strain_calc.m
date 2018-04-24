% calculate 2D strain directly from a regular grid of displacement data
% very simple processing with no filtering or local fitting ahead of strain calc
% assumes dispalcements are in matrix variables um and vm, spacing 16

gridsize = 16;	% for the dvc_test run

[ux,uy] = gradient(um,gridsize,gridsize);
[vx,vy] = gradient(vm,gridsize,gridsize);

% infitesimal (linear) strain calculation
exx = ux;
eyy = vy;
exy = 0.5*(uy+vx);

% add finite (Lagrangian) strain terms

exx = exx + 0.5*(ux.^2 + vx.^2);
eyy = eyy + 0.5*(uy.^2 + vy.^2);
exy = exy + 0.5*(ux.*uy + vx.*vy);

exx(isnan(exx))=0; 
eyy(isnan(eyy))=0;
exy(isnan(exy))=0;
    

