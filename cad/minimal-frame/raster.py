import numpy as np
def rasterize(triangles, colours, camera=(0.6,-1,0.8), width=1250, height=1150):
    # A depth buffer avoids mplot3d's per-face painter-order artifacts.
    forward=np.array(camera,dtype=float);forward/=np.linalg.norm(forward)
    right=np.cross([0,1,0],forward);right/=np.linalg.norm(right)
    up=np.cross(forward,right)
    projected=triangles@np.stack([right,up,forward],axis=1)
    flat=projected.reshape(-1,3)
    low=flat[:,:2].min(axis=0);high=flat[:,:2].max(axis=0)
    scale=min((width-60)/(high[0]-low[0]),(height-60)/(high[1]-low[1]))
    projected[:,:,0]=(projected[:,:,0]-(low[0]+high[0])/2)*scale+width/2
    projected[:,:,1]=height/2-(projected[:,:,1]-(low[1]+high[1])/2)*scale
    image=np.empty((height,width,3),dtype=np.uint8);image[:]=[243,241,234]
    depth=np.full((height,width),-np.inf)
    for tri,colour in zip(projected,colours):
        x0=max(0,int(np.floor(tri[:,0].min())));x1=min(width-1,int(np.ceil(tri[:,0].max())))
        y0=max(0,int(np.floor(tri[:,1].min())));y1=min(height-1,int(np.ceil(tri[:,1].max())))
        if x1<x0 or y1<y0: continue
        x,y=np.meshgrid(np.arange(x0,x1+1)+.5,np.arange(y0,y1+1)+.5)
        a,b,c=tri
        denom=(b[1]-c[1])*(a[0]-c[0])+(c[0]-b[0])*(a[1]-c[1])
        if abs(denom)<1e-8: continue
        u=((b[1]-c[1])*(x-c[0])+(c[0]-b[0])*(y-c[1]))/denom
        v=((c[1]-a[1])*(x-c[0])+(a[0]-c[0])*(y-c[1]))/denom
        w=1-u-v;z=u*a[2]+v*b[2]+w*c[2]
        region=depth[y0:y1+1,x0:x1+1];mask=(u>=0)&(v>=0)&(w>=0)&(z>region)
        region[mask]=z[mask];image[y0:y1+1,x0:x1+1][mask]=(colour*255).astype(np.uint8)
    return image
