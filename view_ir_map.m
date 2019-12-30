V_superlu = csvread("build/V_mat.csv");
v_lower= reshape(V_superlu(7775:38377), 101,303);
min(v_lower(:))
surf(v_lower,'linestyle','none');
