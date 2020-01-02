
close all ;clear ;
figure;
J  = csvread("build/J.csv");
xmin = min(J(:,1));
xmax = max(J(:,1));
ymax = max(J(:,2));
ymin = min(J(:,2));

[xq,yq] = meshgrid(xmin:1:xmax,ymin:1:ymax);
vq = griddata(J(:,1),J(:,2),J(:,3),xq,yq);
surf(vq,'linestyle','none');
cb1 = colorbar;
colormap(jet(256))
xlabel("Width")
ylabel("Height")
cb1.Label.String = "Current(A)"

figure;
V  = csvread("build/V.csv");
xmin = min(V(:,1));
xmax = max(V(:,1));
ymax = max(V(:,2));
ymin = min(V(:,2));

[xq,yq] = meshgrid(xmin:1:xmax,ymin:1:ymax);
vq = griddata(V(:,1),V(:,2),V(:,3),xq,yq);
surf(1.1-vq,'linestyle','none');
colormap(jet(256))
xlabel("Width")
ylabel("Height")
cb = colorbar;
cb.Label.String = "IR drop(V)"

disp("max drop is: ")
1.1 - min(V(:,3))