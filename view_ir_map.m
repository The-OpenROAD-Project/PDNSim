V_superlu = csvread("V_mat.csv");
v_lower= reshape(V_superlu(196:1876),41,41);
min(v_lower(:))
surf(v_lower,'linestyle','none');
