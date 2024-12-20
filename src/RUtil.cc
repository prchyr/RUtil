/*
copyright s. prohira and the T576 collaboration 2018
released under the GNU General Public License version 3
*/

#include "RUtil.hh"

static int fN=1;
static int fNSpec=1.;
static int fNi=1;
static int fNc=1;
static int fNci=1;
static int fTypeSine=4;
static int fTypeCosine=0;
//TVirtualFFT::SetTransform(0);
static TVirtualFFT * fftr2c=TVirtualFFT::FFT(1, &fN, "R2C ES");
static TVirtualFFT * fftc2cF=TVirtualFFT::FFT(1, &fNc, "C2CF ES");
static TVirtualFFT * fftc2cB=TVirtualFFT::FFT(1, &fNci, "C2CB ES");
static TVirtualFFT * sineXfrm=TVirtualFFT::SineCosine(1, &fN, &fTypeSine, "ES");
static TVirtualFFT * cosineXfrm=TVirtualFFT::SineCosine(1, &fN, &fTypeCosine, "ES");
static TVirtualFFT * fftr2cSpec=TVirtualFFT::FFT(1, &fNSpec, "R2C ES");
static TVirtualFFT * fftc2r=TVirtualFFT::FFT(1, &fN, "C2R ES");
static TGraph2D *fXfrmGr2D = new TGraph2D();
static TGraph *fXfrmGr = new TGraph();
static TGraph *sineXfrmGr = new TGraph();
static TGraph *fPSDGr = new TGraph();


/***************FFT things************************

the functions here utilise the above static members for the FFT.
FFTW calculates the fastest and most optimal way to do
an FFT the first time it is initialized with new parameters. after that,
each subsequent fft of the same size is very fast. 

NORMALIZATION:

FFTW doesn't normalize their FFTs, meaning that if you take a signal, FFT it, then IFFT it, it will be scaled by a factor of N. the question is, what is the right way to normalize the signal? 

we use Parseval's theorem to do this:

\sum_0^N V^2 = (1/N) \sum_0^N |H|^2 

where V are the input voltages, and H are the complex output values. for this to hold, given the way FFTW calculates a real-valued to complex-valued FFT, the normalization constant is sqrt(2./N). The overall normalization for power must be 2/N, because FFTW only calculates the positive frequencies (for speed, since for real-valued signals, the negative freqs are the same as the positive) so you need a factor of 2 in there. and the 1/N comes from Parseval's. 


**************************************************/

// static void init(){

// }

/*

the main FFT. returns a TGraph2D with x-axis=frequency, y-axis=real, 
and z-axis=imaginary. 

*/
TGraph2D * RUtil::FFT::fft(TGraph * inGr){

  int n = inGr->GetN();
  if(n!=fN){
    fN=n;
    fftr2c=TVirtualFFT::FFT(1, &fN, "R2C P");
  }

  double dt = inGr->GetX()[50]-inGr->GetX()[49];
  double fs = 1./dt;
  
  fftr2c->SetPoints(inGr->GetY());
  fftr2c->Transform();
  
  double re[n], im[n];
  fftr2c->GetPointsComplex(re, im);

  double df=fs/(n);

  double norm=sqrt(2./(double)n);
  
  double *arr=makeIndices(n, df);
  TGraph2D *outGr=new TGraph2D(n, arr, &re[0], &im[0]);
  for (int i=0;i<outGr->GetN()/2;i++){
    outGr->SetPoint(i, arr[i], re[i]*norm,im[i]*norm);
    outGr->SetPoint(outGr->GetN()/2 +i, arr[outGr->GetN()/2+i], re[outGr->GetN()/2-i]*norm, im[outGr->GetN()/2-i]*norm);
    //    outGr->GetY()[i] *= norm;
    //outGr->GetZ()[i] *= norm;
  }
  delete arr;
  //delete TVirtualFFT::GetCurrentTransform();
  //TVirtualFFT::SetTransform(0);
  *fXfrmGr2D=*outGr;
  delete outGr;
  return fXfrmGr2D;
}

// void RUtil::FFT::fft(TGraph * inGr, TGraph2D outGr){

//   int n = inGr->GetN();
//   if(n!=fN){
//     fN=n;
//     fftr2c=TVirtualFFT::FFT(1, &fN, "R2C P");
//   }

//   double dt = inGr->GetX()[50]-inGr->GetX()[49];
//   double fs = 1./dt;
  
//   fftr2c->SetPoints(inGr->GetY());
//   fftr2c->Transform();
  
//   double re[n], im[n];
//   fftr2c->GetPointsComplex(re, im);

//   double df=fs/(n);

//   double norm=sqrt(2./(double)n);
  

//   for(int i=0;i<n;i++){
//     outGr->SetPoint(outGr->GetN(), (double)i*df, re[i]*norm, im[i]*norm);
//   }

// }

int RUtil::FFT::fft(int n, double * in, complex<double> *out){
  //  int n = inGr->GetN();
  if(n!=fN){
    fN=n;
    fftr2c=TVirtualFFT::FFT(1, &fN, "R2C P");
  }


  for(int i=0;i<n;i++){  
    fftr2c->SetPoint(i,in[i]);
  }
  fftr2c->Transform();
  
  double re[n], im[n];
  fftr2c->GetPointsComplex(re, im);


  double norm=sqrt(2./(double)n);
  


  for (int i=0;i<n;i++){
    out[i]= (re[i]*norm, im[i]*norm);
  }

  return 1;
}

int RUtil::FFT::cfft(int n, complex<double> * inVec, complex<double> * outVec){

  //  int n = inVec.size();
  
  //  complex<double> out[n];
  //cout<<"one"<<endl;
  if(n!=fNc){
    fNc=n;
    fftc2cF=TVirtualFFT::FFT(1, &fNc, "C2CF P");
  }
  
  //  cout<<"two"<<endl;
  for(int i=0;i<n;i++){
    
    fftc2cF->SetPoint(i, inVec[i].real(), inVec[i].imag());

  }
  //cout<<"three"<<endl;
  fftc2cF->Transform();
  //cout<<"four"<<endl;
  double re[n], im[n];
  fftc2cF->GetPointsComplex(re, im);


  double norm=sqrt(2./(double)n);
  
  //cout<<"here"<<endl;
  for (int i=0;i<n;i++){
    auto val=complex<double>(re[i]*norm, im[i]*norm);
    //    cout<<val<<" ";
    outVec[i]=val;
    //cout<<outVec<<" ";
  }
  return 1;
}

int RUtil::FFT::cifft(int n, complex<double> * inVec, complex<double> * outVec){

  //int n = inVec.size();
  
  //  auto out=new TVec1D<complex<double>>(n, 0.);
  
  if(n!=fNci){
    fNci=n;
    fftc2cB=TVirtualFFT::FFT(1, &fNci, "C2CB P");
  }

  for(int i=0;i<n;i++){
    fftc2cB->SetPoint(i, inVec[i].real(), inVec[i].imag());
  }

  fftc2cB->Transform();

  double re[n], im[n];
  fftc2cB->GetPointsComplex(re, im);


  double norm=sqrt(2./(double)n);
  

  for (int i=0;i<n;i++){
    outVec[i]=complex<double>(re[i]*norm, im[i]*norm);

  }

  //delete temp;
  return 1;
}


TGraph * RUtil::FFT::sineTransform(TGraph * inGr){

  int n = inGr->GetN();
  if(n!=fN){
    fN=n;
    sineXfrm=TVirtualFFT::SineCosine(1, &fN, &fTypeSine,"P");
  }

  double dt = inGr->GetX()[50]-inGr->GetX()[49];
  double fs = 1./dt;
  
  sineXfrm->SetPoints(inGr->GetY());
  sineXfrm->Transform();
  
  double re[n], im[n];
  //sineXfrm->GetPointsComplex(re, im);
  sineXfrm->GetPoints(re);

  double df=fs/(2.*n);

  double norm=.5*sqrt(2./(double)n);
  
  double *arr=makeIndices(n, df);
  TGraph *outGr=new TGraph(n, arr, &re[0]);
  for (int i=0;i<outGr->GetN();i++){
    outGr->GetY()[i] *= norm;
  }
  delete arr;
  //delete TVirtualFFT::GetCurrentTransform();
  //TVirtualFFT::SetTransform(0);
  *sineXfrmGr=*outGr;
  delete outGr;
  return sineXfrmGr;
}

double * RUtil::FFT::sineTransform(int n, double * in){


  if(n!=fN){
    fN=n;
    sineXfrm=TVirtualFFT::SineCosine(1, &fN, &fTypeSine,"P");
  }

  double dt = in[50]-in[49];
  double fs = 1./dt;
  
  sineXfrm->SetPoints(in);
  sineXfrm->Transform();
  
  double re[n], im[n];
  //sineXfrm->GetPointsComplex(re, im);
  sineXfrm->GetPoints(re);

  double df=fs/(2.*n);

  double norm=.5*sqrt(2./(double)n);
  

  double *out= new double[n];
  for (int i=0;i<n;i++){
    out[i]=re[i]*norm;
  }



  return out;
}

/*

Provide a TGraph 2D to this function and it will return the inverse fft
in the form of a tgraph.

 */


TGraph * RUtil::FFT::ifft(TGraph2D * inGr){
  int n = inGr->GetN();
  if(n!=fNi){
    fNi=n;
    //    fftr2c=TVirtualFFT::FFT(1, &fN, "R2C P");
    fftc2r=TVirtualFFT::FFT(1, &fNi, "C2R P K");
  }
  
  double df = inGr->GetX()[1]-inGr->GetX()[0];
  double fs=(double)n*df;
  double dt = 1./fs;
  
  fftc2r->SetPointsComplex(inGr->GetY(), inGr->GetZ());
  fftc2r->Transform();
  
  double re[n];
  fftc2r->GetPoints(re);

  double norm=sqrt(1./(2.*(double)n));

  double * arr=makeIndices(n, dt);
  TGraph *outGr=new TGraph(n, arr, re);

  for (int i=0;i<outGr->GetN();i++) outGr->GetY()[i] *= norm;
  *fXfrmGr=*outGr;
  delete arr;
  delete outGr;
  return fXfrmGr;
}

int RUtil::FFT::ifft(int n, complex<double> * in, double *out){
  //  int n = inGr->GetN();
  if(n!=fNi){
    fNi=n;
    fftc2r=TVirtualFFT::FFT(1, &fNi, "C2R P");
  }


  for(int i=0;i<n;i++){  
    fftc2r->SetPoint(i,in[i].real(), in[i].imag());
  }
  fftc2r->Transform();
  
  double re[n];
  fftc2r->GetPoints(re);


  double norm=sqrt(2./(double)n);
  


  for (int i=0;i<n;i++){
    out[i]= (re[i]*norm);
  }

  return 1;
}

TGraph * RUtil::FFT::convolve(TGraph *inGr1, TGraph *inGr2){
  auto fft1=(TGraph2D*)RUtil::FFT::fft(inGr1)->Clone();
  fft1->SetName("one");
  //  auto flipped=RUtil::flip(inGr2);
  auto fft2=(TGraph2D*)RUtil::FFT::fft(inGr2)->Clone();
  fft2->SetName("two");
  //  RUtil::FFT::fftshift(fft1);
  //RUtil::FFT::fftshift(fft2);
  auto conv=RUtil::multComplex(fft1,fft2);
  auto inv=(TGraph*)RUtil::FFT::ifft(conv)->Clone();
  RUtil::FFT::fftshift(inv);
  delete fft1;
  //delete flipped;
  delete fft2;
  delete conv;
  return inv;
}

void RUtil::FFT::fftshift(int N, complex<double>* in){
  for(int j=N/2;j<N;j++){
     auto temp=in[j];
     in[j]=in[j-(N/2)];
     in[j-(N/2)]=temp;
  }
}

void RUtil::FFT::fftshift(TGraph2D * in){
  auto N=in->GetN();
  for(int j=N/2;j<N;j++){
    auto tempY=in->GetY()[j];
    auto tempZ=in->GetZ()[j];
    in->SetPoint(j, in->GetX()[j], in->GetY()[j-(N/2)], in->GetZ()[j-(N/2)]);
    in->SetPoint(j-(N/2), in->GetX()[j-(N/2)], tempY, tempZ);   
    //     in[j]=in[j-(N/2)];
    //in[j-(N/2)]=temp;
  }
}

void RUtil::FFT::fftshift(TGraph * in){
  auto N=in->GetN();
  for(int j=N/2;j<N;j++){
    auto tempY=in->GetY()[j];
    in->SetPoint(j, in->GetX()[j], in->GetY()[j-(N/2)]);
    in->SetPoint(j-(N/2), in->GetX()[j-(N/2)], tempY);   
    //     in[j]=in[j-(N/2)];
    //in[j-(N/2)]=temp;
  }
}



/*

psd, returns a tgraph in dbm/hz from an input tgraph

*/
//assumes a 50ohm system
TGraph * RUtil::FFT::psd(TGraph * inGr, int dbFlag, double rBW ){
  auto xfrm=fft(inGr);
  int n=xfrm->GetN();
  //  double norm=1./(double)sqrt(n);
  auto xx=xfrm->GetX();
  auto re=xfrm->GetY();
  auto im=xfrm->GetZ();
  double yy[n];

  //resolution bandwidth defaults to nyquist
  rBW=rBW==0?xx[n-1]/2.:rBW;
  switch (dbFlag){
  case 0:
    yy[0]=re[0]/50.;
    break;
  case 1:
    yy[0]=vToDbmHz(rBW,re[0]);
    break;
  case 2:
    yy[0]=vToDbmGHz(rBW,re[0]);
  default:
    yy[0]=re[0]/50.;
    break;
  }
    //yy[0]=dbFlag==1?vToDbmHz(rBW,re[0]):re[0]/50.;
    //yy[0]=dbFlag==2?vToDbmGHz(rBW,re[0]):re[0]/50.;
  for(int i=1;i<(n+1)/2;i++){
    // yy[i]=vToDbmHz(rBW, re[i], im[i]);
    switch (dbFlag){
    case 0:
      yy[i]=(re[i]*re[i]+im[i]*im[i])/50.;
      break;
    case 1:
      yy[i]=vToDbmHz(rBW,re[i], im[i]);
      break;
    case 2:
      yy[i]=vToDbmGHz(rBW,re[i], im[i]);
      break;
    }
  }
  switch (dbFlag){
  case 0:
    yy[n/2]=(re[n/2]*re[n/2]+im[n/2]*im[n/2])/50.;
    break;
  case 1:
    yy[n/2]=vToDbmHz(rBW,re[n/2], im[n/2]);
    break;
  case 2:
    yy[n/2]=vToDbmGHz(rBW,re[n/2], im[n/2]);
    break;
  }

  TGraph * outGr=new TGraph((n/2), xx, yy);
    outGr->SetTitle("psd");
  outGr->SetName("psd");
  *fPSDGr=*outGr;
  //delete outGr;
  return outGr;//fPSDGr;
}



TGraph * RUtil::FFT::hilbertTransform(TGraph *inGr){
  auto infft=fft(inGr);
  int n=infft->GetN();

  for(int i=0;i<n;i++){
    double im=infft->GetZ()[i];
    infft->GetZ()[i]=infft->GetY()[i];
    infft->GetY()[i]=-im;
  }

  // for(int i=0;i<n/2;i++){
  //   double x=infft->GetY()[i];
  //   double y=infft->GetZ()[i];
  //   double r=sqrt((x*x)+(y*y));
  //   double phi=TMath::ATan2(y,x);
  //   phi-=RUtil::pi/2.;
  //   x=r*cos(phi);
  //   y=r*sin(phi);
  //   infft->GetY()[i]=x;
  //   infft->GetZ()[i]=y;
  // }

  // for(int i=n/2;i<n;i++){
  //   double x=infft->GetY()[i];
  //   double y=infft->GetZ()[i];
  //   double r=sqrt((x*x)+(y*y));
  //   double phi=TMath::ATan2(y,x);
  //   phi+=RUtil::pi/2.;
  //   x=r*cos(phi);
  //   y=r*sin(phi);
  //   infft->GetY()[i]=x;
  //   infft->GetZ()[i]=y;
  // }
  auto outGr=ifft(infft);
  return outGr;
}

TGraph * RUtil::FFT::hilbertEnvelope(TGraph * inGr){
  auto hilb=hilbertTransform(inGr);
  TGraph * out=new TGraph();
  for(int i=0;i<hilb->GetN();i++){
    out->SetPoint(i, inGr->GetX()[i], sqrt((inGr->GetY()[i]*inGr->GetY()[i])+(hilb->GetY()[i]*hilb->GetY()[i])));
  }
  //  *fXfrmGr=*out;
  //  delete out;
  return out;//fXfrmGr;
}

/*
works well so long as your frequency of interest is an exact bin. 
e.g. need to have the right number of samples in the graph such that
there is a bin right at your freq of interest. (usually just an 
even number of samples)


 */


TGraph * RUtil::FFT::zeroPhaseAt(TGraph * inGr, double freq, int debug){
  auto fftGr=RUtil::FFT::fft(inGr);
  double binwidth=fftGr->GetX()[10]-fftGr->GetX()[9];
  double thresh=binwidth/2.5;
  int index=0;
  double thisFreq=0.;
  double lastFreq=0.;
  for(index=0;index<fftGr->GetN();index++){
    thisFreq=fftGr->GetX()[index];
    if(abs(thisFreq-freq)<thresh)break;
    //if(thisFreq==freq)break;
    lastFreq=thisFreq;
  }
  if(debug==1){
    cout<<fftGr->GetX()[index]<<endl;
  }
  double mag=sqrt((fftGr->GetY()[index]*fftGr->GetY()[index])+(fftGr->GetZ()[index]*fftGr->GetZ()[index]));
  fftGr->GetY()[index]=mag;
  fftGr->GetZ()[index]=0.;
  auto  outGr=(TGraph*)RUtil::FFT::ifft(fftGr)->Clone();
  return outGr;
  
}

TGraph * RUtil::FFT::setPhaseAt(TGraph * inGr, double freq, double phaseAng, int debug){
  auto fftGr=RUtil::FFT::fft(inGr);
  double binwidth=fftGr->GetX()[10]-fftGr->GetX()[9];
  double thresh=binwidth/2.5;
  int index=0;
  double thisFreq=0.;
  double lastFreq=0.;
  for(index=0;index<fftGr->GetN();index++){
    thisFreq=fftGr->GetX()[index];
    if(abs(thisFreq-freq)<thresh)break;
    //if(thisFreq==freq)break;
    lastFreq=thisFreq;
  }
  if(debug==1){
    cout<<fftGr->GetX()[index]<<endl;
  }
  double mag=sqrt((fftGr->GetY()[index]*fftGr->GetY()[index])+(fftGr->GetZ()[index]*fftGr->GetZ()[index]));
  fftGr->GetY()[index]=mag*sin(phaseAng);
  fftGr->GetZ()[index]=mag*cos(phaseAng);
  auto  outGr=(TGraph*)RUtil::FFT::ifft(fftGr)->Clone();
  return outGr;
  
}



TGraph* RUtil::FFT::peakFreqGraph(TGraph *gr, Int_t binsize , Int_t overlap, Int_t zero_pad_length, int win_type, double thresh){
  Int_t size = gr->GetN();
  double dt=(gr->GetX()[1]-gr->GetX()[0]); 
  double samprate=1./dt;
  double xmax = gr->GetX()[gr->GetN()-1];
  double xmin = gr->GetX()[0];
  zero_pad_length=zero_pad_length<=binsize?binsize:zero_pad_length;
  Int_t num_zeros=(zero_pad_length-binsize)/2;
  //  Int_t nbins = size/overlap;
  int nbins=size/(binsize-overlap);
  //xmax=((double)nbins*((double)binsize-(double)overlap))/samprate;
  char*timebuff;
  //double samplerate = size/(xmax-xmin);
  double bandwidth = 1e9*samprate;
  // if(zero_pad_length!=fNSpec){
  //   fNSpec=zero_pad_length;
  //   fftr2cSpec=TVirtualFFT::FFT(1, &fNSpec, "R2C P K");
  // }
  TGraph * in=new TGraph(zero_pad_length);
  TGraph * outt=new TGraph(zero_pad_length);

  vector<double> sX, sY, sZ;
  Int_t start = 0;
  auto timeH=0.;
  auto lasttimeH=0.;
  //  Int_t j=0;
  //cout<<size<<" "<<nbins<<" "<<zero_pad_length<<" "<<binsize<<" "<<overlap<<" "<<num_zeros<<" "<<xmax*samprate<<endl;
  TGraph * outGr=new TGraph();
  
  for(int i=0;i<nbins;i++){
    if(start+binsize-1>0){
      int sampnum=0;
      for(int j=0;j<zero_pad_length;j++){
	if(j<num_zeros||j>=binsize+num_zeros){
	  in->SetPoint(j, gr->GetX()[j], 0.);
	}
	else if(j>=num_zeros&&j<binsize+num_zeros){
	  if(sampnum+start<size){
	    in->SetPoint(j, gr->GetX()[j], gr->GetY()[sampnum+start]*window(sampnum, binsize, win_type));
	  }
	  else{
	    in->SetPoint(j, gr->GetX()[j], 0.);
	  }
	  sampnum++;
	}
      }
      timeH=gr->GetX()[start+(sampnum/2)];
      
      outt=RUtil::FFT::psd(in,0, samprate/2.);
      
      auto maxx=RUtil::maxInRange(outt, 0, 3);
      if(timeH>lasttimeH && maxx>thresh){
	outGr->SetPoint(outGr->GetN(), timeH, RUtil::locMaxInRange(outt, 0, 3.));
	}
      lasttimeH=timeH;
    }
    start+=(binsize-overlap);
   
  }

  outGr->SetTitle("");
  outGr->SetName("Peak Frequency");
  outGr->GetXaxis()->SetTitle("Time (ns)");
  outGr->GetYaxis()->SetTitle("Frequency (GHz)");
  

  outt->Delete();
  in->Delete();

  // spectrogramGr->Delevte();
  return outGr;
  //return outdat;
}





TGraph * RUtil::FFT::plotPhase(TGraph *inGr){
  auto fftGr=RUtil::FFT::fft(inGr);
  auto ph=vector<double>();
  for(int i=0;i<fftGr->GetN()/2;i++){
    ph.push_back(TMath::ATan2(fftGr->GetZ()[i], fftGr->GetY()[i]));
  }
  auto outGr=new TGraph(ph.size(), fftGr->GetX(), &ph[0]);
  return outGr;
}

double RUtil::FFT::getPhaseAt(TGraph *inGr, double freq){

  auto fftGr=RUtil::FFT::fft(inGr);
  int index=RUtil::getIndex(fftGr, freq);
  auto ph=TMath::ATan2(fftGr->GetZ()[index], fftGr->GetY()[index]);
  //  cout<<index<<" "<<fftGr->GetX()[index]<<endl;
  //  delete fftGr;

  return ph;
}

double RUtil::FFT::getMagAt(TGraph *inGr, double freq){
  auto fftGr=RUtil::FFT::fft(inGr);
  int index=RUtil::getIndex(fftGr, freq);
  auto mag=sqrt(fftGr->GetY()[index]*fftGr->GetY()[index] + fftGr->GetZ()[index]*fftGr->GetZ()[index]);
  //  cout<<index<<" "<<fftGr->GetX()[index]<<endl;
  //  delete fftGr;
  return mag;
}



// TGraph * RUtil::FFT::phasorTransform(TGraph *inGr){
//   auto infft=fft(inGr);
//   int n=infft->GetN();

//   for(int i=0;i<n;i++){
//     double im=infft->GetZ()[i];
//     infft->GetZ()[i]=0;//infft->GetY()[i];
//     infft->GetY()[i]=infft->GetY()[i]+im;
//   }
//   auto outGr=ifft(infft);
//   return outGr;
// }

TH2D* RUtil::FFT::spectrogram(TGraph *gr, Int_t binsize , Int_t overlap, Int_t zero_pad_length, int win_type, int dbFlag, double ymin, double ymax){
  Int_t size = gr->GetN();
  double dt=(gr->GetX()[1]-gr->GetX()[0]); 
  double samprate=1./dt;
  double xmax = gr->GetX()[gr->GetN()-1];
  double xmin = gr->GetX()[0];
  zero_pad_length=zero_pad_length<=binsize?binsize:zero_pad_length;
  double scale=sqrt((double)zero_pad_length/(double)binsize);
  Int_t num_zeros=(zero_pad_length-binsize)/2;
  //  Int_t nbins = size/overlap;
  int nbins=size/(binsize-overlap);
  //xmax=((double)nbins*((double)binsize-(double)overlap))/samprate;
  char*timebuff;
  //double samplerate = size/(xmax-xmin);
  double bandwidth = 1e9*samprate;
  // if(zero_pad_length!=fNSpec){
  //   fNSpec=zero_pad_length;
  //   fftr2cSpec=TVirtualFFT::FFT(1, &fNSpec, "R2C P K");
  // }
  


  vector<double> sX, sY, sZ;
  Int_t start = 0;
  //  Int_t j=0;
  //cout<<size<<" "<<nbins<<" "<<zero_pad_length<<" "<<binsize<<" "<<overlap<<" "<<num_zeros<<" "<<xmax*samprate<<endl;
  TH2D *spectrogramHist=new TH2D("", "", nbins-1, xmin, xmax, (zero_pad_length), 0, samprate);
  spectrogramHist->SetDirectory(0);
  for(int i=0;i<nbins;i++){
    TGraph * in=new TGraph(zero_pad_length);
    if(start+binsize-1>0){
      int sampnum=0;
      for(int j=0;j<zero_pad_length;j++){
	if(j<num_zeros||j>=binsize+num_zeros){
	  in->SetPoint(j, gr->GetX()[j], 0.);
	}
	else if(j>=num_zeros&&j<binsize+num_zeros){
	  if(sampnum+start<size){
	    in->SetPoint(j, gr->GetX()[j], gr->GetY()[sampnum+start]*window(sampnum, binsize, win_type)*scale);
	  }
	  else{
	    in->SetPoint(j, gr->GetX()[j], 0.);
	  }
	  sampnum++;
	}
      }

	//   for(int j=0;j<num_zeros;j++){
      // 	//if((j+start)>=size)break;
      // 	//    for(int j = 0;j<=zero_pad_length;j++){
      // 	in->SetPoint(j, gr->GetX()[j], 0.);
      // }
      // for(int j=num_zeros;j<binsize+num_zeros;j++){
      // 	//      if((j+start)>=size)break;
      // 	in->SetPoint(j, gr->GetX()[j], gr->GetY()[j+start]*window(j-num_zeros, binsize, win_type));//
      // }
      // for(int j=binsize+num_zeros;j<zero_pad_length;j++){
      // 	//if((j+start)>=size)break;
      // 	in->SetPoint(j, gr->GetX()[j], 0);
      // }
      
      auto outt=RUtil::FFT::psd(in, dbFlag,samprate/2.);
      
      for(int j = 0;j<outt->GetN();j++){
	Double_t z = outt->GetY()[j];
	if(!isfinite(z))z=0.;
	spectrogramHist->SetBinContent(i,j,z);//dbm/hz
	//sX.push_back(i*binsize*dt);
	//sY.push_back(outt->GetX()[j]);
	//sZ.push_back(z);
      }
      delete outt;
      delete in;
      //    cout<<sX.size()<<endl;
    }
      start+=(binsize-overlap);
    }
  //  cout<<dbFlag<<endl;

  spectrogramHist->GetYaxis()->SetRangeUser(0, spectrogramHist->GetYaxis()->GetXmax()/2.1);
  spectrogramHist->GetXaxis()->SetTitle("Time [ns]");
  spectrogramHist->GetYaxis()->SetTitle("Frequency [GHz]");
  if(dbFlag==1){
    spectrogramHist->GetZaxis()->SetTitle("dBm Hz^{-1}");
  }
  else if (dbFlag==2){
    spectrogramHist->GetZaxis()->SetTitle("dBm GHz^{-1}");
  }
  else{
    spectrogramHist->GetZaxis()->SetTitle("W GHz^{-1}");
  }
  
  spectrogramHist->GetZaxis()->SetTitleOffset(1.5);
  spectrogramHist->GetXaxis()->SetTitleSize(.05);
  spectrogramHist->GetYaxis()->SetTitleSize(.05);
  spectrogramHist->GetZaxis()->SetTitleSize(.05);
  spectrogramHist->GetXaxis()->SetLabelSize(.05);
  spectrogramHist->GetYaxis()->SetLabelSize(.05);
  spectrogramHist->GetZaxis()->SetLabelSize(.05);
  spectrogramHist->GetYaxis()->SetTitleOffset(1.1);
  spectrogramHist->GetYaxis()->SetRangeUser(ymin, ymax);
  //  delete(in);
  //  delete(outt);


  // spectrogramGr->Delevte();
  return spectrogramHist;
  //return outdat;
}


TGraph2D* RUtil::FFT::spectrogramGraph(TGraph *gr, Int_t binsize , Int_t overlap, Int_t zero_pad_length, int win_type, int dbFlag, double ymin, double ymax){
  Int_t size = gr->GetN();
  double dt=(gr->GetX()[1]-gr->GetX()[0]); 
  double samprate=1./dt;
  double xmax = gr->GetX()[gr->GetN()-1];
  double xmin = gr->GetX()[0];
  zero_pad_length=zero_pad_length<=binsize?binsize:zero_pad_length;
  double scale=sqrt((double)zero_pad_length/(double)binsize);
  Int_t num_zeros=(zero_pad_length-binsize)/2;
  //  Int_t nbins = size/overlap;
  int nbins=size/(binsize-overlap);
  //xmax=((double)nbins*((double)binsize-(double)overlap))/samprate;
  //  char*timebuff;
  //double samplerate = size/(xmax-xmin);
  double bandwidth = 1e9*samprate;
  // if(zero_pad_length!=fNSpec){
  //   fNSpec=zero_pad_length;
  //   fftr2cSpec=TVirtualFFT::FFT(1, &fNSpec, "R2C P K");
  // }
  


  vector<double> sX, sY, sZ;
  Int_t start = 0;
  //  Int_t j=0;
  //cout<<size<<" "<<nbins<<" "<<zero_pad_length<<" "<<binsize<<" "<<overlap<<" "<<num_zeros<<" "<<xmax*samprate<<endl;
  TGraph2D *spectrogramGraph=new TGraph2D();
  spectrogramGraph->SetDirectory(0);
  for(int i=0;i<nbins;i++){
    TGraph * in=new TGraph(zero_pad_length);
    if(start+binsize-1>0){
      int sampnum=0;
      for(int j=0;j<zero_pad_length;j++){
	if(j<num_zeros||j>=binsize+num_zeros){
	  in->SetPoint(j, gr->GetX()[j], 0.);
	}
	else if(j>=num_zeros&&j<binsize+num_zeros){
	  if(sampnum+start<size){
	    in->SetPoint(j, gr->GetX()[j], gr->GetY()[sampnum+start]*window(sampnum, binsize, win_type)*scale);
	  }
	  else{
	    in->SetPoint(j, gr->GetX()[j], 0.);
	  }
	  sampnum++;
	}
      }

	//   for(int j=0;j<num_zeros;j++){
      // 	//if((j+start)>=size)break;
      // 	//    for(int j = 0;j<=zero_pad_length;j++){
      // 	in->SetPoint(j, gr->GetX()[j], 0.);
      // }
      // for(int j=num_zeros;j<binsize+num_zeros;j++){
      // 	//      if((j+start)>=size)break;
      // 	in->SetPoint(j, gr->GetX()[j], gr->GetY()[j+start]*window(j-num_zeros, binsize, win_type));//
      // }
      // for(int j=binsize+num_zeros;j<zero_pad_length;j++){
      // 	//if((j+start)>=size)break;
      // 	in->SetPoint(j, gr->GetX()[j], 0);
      // }
      
      auto outt=RUtil::FFT::psd(in, dbFlag,samprate/2.);
      
      for(int j = 0;j<outt->GetN();j++){
	Double_t z = outt->GetY()[j];
	if(!isfinite(z))z=0.;
	spectrogramGraph->SetPoint(spectrogramGraph->GetN(), gr->GetX()[start],outt->GetX()[j],z);//dbm/hz
	//sX.push_back(i*binsize*dt);
	//sY.push_back(outt->GetX()[j]);
	//sZ.push_back(z);
      }
      delete outt;
      delete in;
      //    cout<<sX.size()<<endl;
    }
      start+=(binsize-overlap);
    }
  //  cout<<dbFlag<<endl;

  // spectrogramGraph->GetYaxis()->SetRangeUser(0, spectrogramGraph->GetYaxis()->GetXmax()/2.1);
  // spectrogramGraph->GetXaxis()->SetTitle("Time [ns]");
  // spectrogramGraph->GetYaxis()->SetTitle("Frequency [GHz]");
  // if(dbFlag==1){
  //   spectrogramGraph->GetZaxis()->SetTitle("dBm Hz^{-1}");
  // }
  // else if (dbFlag==2){
  //   spectrogramGraph->GetZaxis()->SetTitle("dBm GHz^{-1}");
  // }
  // else{
  //   spectrogramGraph->GetZaxis()->SetTitle("W GHz^{-1}");
  // }
  // spectrogramGraph->GetZaxis()->SetTitleOffset(1.5);
  // spectrogramGraph->GetXaxis()->SetTitleSize(.05);
  // spectrogramGraph->GetYaxis()->SetTitleSize(.05);
  // spectrogramGraph->GetZaxis()->SetTitleSize(.05);
  // spectrogramGraph->GetXaxis()->SetLabelSize(.05);
  // spectrogramGraph->GetYaxis()->SetLabelSize(.05);
  // spectrogramGraph->GetZaxis()->SetLabelSize(.05);
  // spectrogramGraph->GetYaxis()->SetTitleOffset(1.1);
  // spectrogramGraph->GetYaxis()->SetRangeUser(ymin, ymax);
  // //  delete(in);
  // //  delete(outt);


  // spectrogramGr->Delevte();
  return spectrogramGraph;
  //return outdat;
}


TH2D * RUtil::FFT::avgSpectrograms(vector<TH2D*> inh){
  TH2D *out = (TH2D*)inh[0]->Clone();
  for(int i=1;i<inh.size();i++){
    if(out->GetNbinsX()==inh[i]->GetNbinsX()&&out->GetNbinsY()==inh[i]->GetNbinsY()){
      out->Add(inh[i]);
    }
  }
  out->Scale(1./inh.size());
  return out;
}

TGraph* RUtil::FFT::real(TGraph2D *ingr){
  auto og=new TGraph(ingr->GetN());
  for(int i=0;i<ingr->GetN();i++){
    og->SetPoint(i, ingr->GetX()[i], ingr->GetY()[i]);
  }
  return og;
}

TGraph* RUtil::FFT::imag(TGraph2D *ingr){
  auto og=new TGraph(ingr->GetN());
  for(int i=0;i<ingr->GetN();i++){
    og->SetPoint(i, ingr->GetX()[i], ingr->GetZ()[i]);
  }
  return og;
}

/*******************SVD things**********************

these functions use the root linear algebra routines, which are 
quite extensive. they are built on the usual GSL routines. 

memory management is not done here.


 */

TVectorD RUtil::SVD::normalize(TVectorD vec){
  auto a=TVectorD(vec);
  auto b=TVectorD(vec);
  int len=vec.GetNrows();
  double norm=0;
  a.Sqr();
  norm=sqrt(a.Sum());
  b*=(1./norm);
  return b;
}


double RUtil::SVD::norm(TVectorD vec){
  auto a=TVectorD(vec);
  int len=vec.GetNrows();
  double norm=0;
  a.Sqr();
  norm=sqrt(a.Sum());
  return norm;
}

TMatrixD RUtil::SVD::eventMatrix(vector<TGraph*> vecs){
  int x=vecs.size();
  int y=vecs[0]->GetN();
  TMatrixD M(x,y);
  M.Zero();
  for(int i=0;i<x;i++){
    for(int j=0;j<y;j++){
    M[i][j]=vecs[i]->GetY()[j];
    }
  }
  return M;
}

//make a density matrix partitioned along D
TMatrixD RUtil::SVD::densityMatrix(TGraph *vec, int D, int xlim){
  int N=xlim==0?vec->GetN():xlim;
  D=D==1?N:D;
  int d=(int) (N/D);
  TVectorD V(vec->GetN(), vec->GetY());
  TMatrixD A(D, D);
  A.Zero();
  for(int i=0;i<d;i++){
    auto vv=V.GetSub(i*D, (i*D)+D, "");
    A.Rank1Update(vv, 1.);
  }
  return A;
}

TMatrixD RUtil::SVD::truncateSVD(TDecompSVD svd, int below, int above){
  auto V=svd.GetV();
  auto S=svd.GetSig();
  auto U=svd.GetU();
  auto temp=TMatrixD(U.GetNcols(), V.GetNcols());
  //cout<<U.GetNcols()<<" "<<V.GetNcols()<<endl;
  //cout<<temp.GetNrows()<<" "<<temp.GetNcols()<<endl;

  temp.Zero();
  for(int i=above;i<below;i++){
    temp[i][i]=S[i];
  }
  //cout<<"here"<<endl;
  auto SV=temp*V.T();
  //cout<<"no here"<<endl;
  //cout<<temp.GetNrows()<<" "<<temp.GetNcols()<<endl;
  auto USV=U*temp;
  //  cout<<"no no here"<<endl;
  return USV;
}


TMatrixD RUtil::SVD::reconstructSingle(TDecompSVD svd, int val){
  auto M = truncateSVD(svd, val+1, val);
  return M;
}

TVectorD RUtil::SVD::toVector(TGraph *vec){
  auto v=TVectorD(vec->GetN(), vec->GetY());
  return v;
}

//TVectorD subtract(

//TVectorD hilbertTransform(TVectorD vec){
  

TVectorD RUtil::SVD::avgVector(TMatrixD m){
  int sizex=m.GetNcols();
  int sizey=m.GetNrows();
  auto vec=TVectorD(sizex);
  //vector<double>vec;
  for(int i=0;i<sizey;i++){
    vec+=m[i];
  }
  return vec*=1./((double)sizey);
}  


TMatrixD RUtil::SVD::buildBasis(TMatrixD m, int num){
  auto rows=m.GetNrows();
  auto cols=m.GetNcols();
  //cout<<rows<<" "<<cols<<endl;
  num=num>cols?cols:num;
  auto M=TMatrixD(num, cols);

  auto mm=rows>=cols?m:m.T();
  //  cout<<mm.GetNrows()<<" "<<mm.GetNcols()<<endl;
  auto svd=TDecompSVD(mm);
  rows=M.GetNrows();
  cols=M.GetNcols();
  //cout<<rows<<" "<<cols<<endl;
  for(int i=0;i<rows;i++){
    auto recon=reconstructSingle(svd,i).T();
    //cout<<recon.GetNrows()<<" "<<recon.GetNcols()<<endl;
    auto avg=avgVector(recon);
    //cout<<avg.GetNrows()<<endl;//" "<<avg.GetNcols()<<endl;
    auto vec=normalize(avg);
    //auto vec=avg;
    //cout<<vec.GetNrows()<<endl;
    for(int j=0;j<cols;j++){
      M[i][j]=vec[j];
    }
  }
  return rows>cols?M.T():M;
}

TGraph* RUtil::SVD::drawBasisVector(TMatrixD M, int row, double dt){
  auto gr=new TGraph();
  auto indices=makeIndices(M.GetNcols(), dt);
  for(int i=0;i<M.GetNcols();i++){
    gr->SetPoint(gr->GetN(), indices[i], M[row][i]);
  }
  return gr;
}

TVectorD RUtil::SVD::getCoefficients(TVectorD V, TMatrixD B){
  int y=B.GetNrows();
  int x=B.GetNcols();
  auto vec=TVectorD(y);
  vec.Zero();
  for (int i=0;i<y;i++){
    auto vnorm=normalize(V);
    for( int j=0;j<x;j++){
      vnorm[j]*=B[i][j];
    }
    vec[i]=vnorm.Sum();
  }
  return vec;
}

TVectorD RUtil::SVD::expandInBasis(TVectorD V, TMatrixD B, int num){

  int y=B.GetNrows();
  int x=B.GetNcols();
  //  x=x>V.GetNrows()?V.GetNrows():x;
  auto vec=getCoefficients(V, B);
  auto outvec=TVectorD(V.GetNrows());
  outvec.Zero();
  num=num>=y?y:num;
  for (int i=0;i<num;i++){
    //    auto aligned=
    for (int j=0;j<x;j++){
      outvec[j]+=(vec[i]*B[i][j]);
    }
  }
  outvec*=norm(V);
  return outvec;
}

TGraph * RUtil::SVD::expandInBasis(TGraph * G, TMatrixD B, int num){
  TVectorD V = toVector(G);
  double dt = G->GetX()[10]-G->GetX()[9];
  return toGraph(expandInBasis(V, B, num), 1./dt, G->GetX()[0]);
}

TGraph * RUtil::SVD::filter(TGraph *G, TMatrixD B, int num){
  TVectorD V = toVector(G);
  auto filter=expandInBasis(V, B, num);
  //  cout<<V.GetNrows()<<" "<<B.GetNcols()<<endl;
  auto oV=V-filter;
  double dt = G->GetX()[10]-G->GetX()[9];
  return toGraph(oV, 1./dt, G->GetX()[0]);
  
}

TVectorD RUtil::SVD::filter(TVectorD V, TMatrixD B, int num){
  auto filter=expandInBasis(V, B, num);
  return V-filter;
}


//must be for square matrix
TMatrixD RUtil::SVD::makeFilter(TDecompSVD svd, int below, int above){
  auto M = truncateSVD(svd, below, above);
  auto D = M.GetNrows();
  auto I = TMatrixD(D, D);
  I.UnitMatrix();
  auto filter=I-(M*(1./M.NormInf()));
  //  auto filter=(M*(1./M.E2Norm()));
  return filter;
}


TGraph * RUtil::SVD::toGraph(TVectorD v, double samplerate, double delay,TString name){
  double tdiv=1./samplerate;
  double *arr=makeIndices(v.GetNrows(), tdiv, delay);
  auto  gg=new TGraph(v.GetNrows(), arr, &v[0]);
  gg->SetTitle("");
  gg->SetName(name);
  gg->GetXaxis()->SetRangeUser(0,gg->GetX()[gg->GetN()-1]);
  delete arr;
  return gg;
}

TVectorD RUtil::SVD::flatten(TMatrixD m){
  int sizex=m.GetNcols();
  int sizey=m.GetNrows();
    auto vec=TVectorD(sizex*sizey);
  //vector<double>vec;
  for(int i=0;i<sizey;i++){
    vec.SetSub(i*sizex, m[i]);
  }
  return vec;
}  

TH2F * RUtil::SVD::matrixMap(TMatrixD M, TString name){
  int sizex=M.GetNrows();
  int sizey=M.GetNcols();
  //  cout<<sizey<<" "<<sizex<<endl;
  TH2F * map=new TH2F(name, name, sizex, 0.,sizex, sizey, 0,sizey);
  for(int i=0;i<sizex;i++){
    for(int j=0;j<sizey;j++){
      map->SetBinContent(i,j, M[i][j]);
    }
  }
  
  return map;
}





/******************utility things*********************

none of these manage their own memory. normaliz() for example, will
make a new tgraph that you'll need to delete on your own.

*/

// RUtilGraph * RUtilGraph::operator*(const double a){
//   for(int i=0;i<this->GetN();i++)this->GetY()[i]*=a;
// }

// double * RUtil::makeIndices(int n, double step, double offset){
//     double *out=new double[n];
//   for(int i=0;i<n;i++){
//     out[i]=((double)i*step+offset);
//   }
//   return out;
// }

double RUtil::toDBPower(double val, double ref){
  return 10*log10(val/ref);
}

double RUtil::toDBField(double val, double ref){
  return 20*log10(val/ref);
}

double RUtil::vToDbmHz(double bandwidthGSs, double re, double im){
  double val=re*re+im*im;
  return (10.*log10(val/50.))+30-(10.*log10(bandwidthGSs*1.e9));
}

double RUtil::vToDbmGHz(double bandwidthGSs, double re, double im){
  double val=re*re+im*im;
  return (10.*log10(val/50.))+30-(10.*log10(bandwidthGSs));
}

TGraph * RUtil::normalize(TGraph * inGr){
    double length=inGr->GetN();
    double norm=0;
    for(int i=0;i<length;i++){
      norm+=inGr->GetY()[i]*inGr->GetY()[i];
    }
    TGraph *og = (TGraph*)inGr->Clone();
    for(int i=0;i<length;i++)og->GetY()[i]/=sqrt(norm);
    return og;
  }

double RUtil::normalize(double * result, int N, double *input){
  double length=N;
  double norm=0;
    for(int i=0;i<length;i++){
      norm+=input[i]*input[i];
    }

    for(int i=0;i<length;i++)result[i]=input[i]/sqrt(norm);
    return sqrt(norm);
  }

TGraph * RUtil::normToPeak(TGraph * inGr){
    double length=inGr->GetN();
    double peak=TMath::MaxElement(inGr->GetN(), inGr->GetY());
    TGraph *og = (TGraph*)inGr->Clone();
    for(int i=0;i<length;i++){
      og->SetPoint(i, inGr->GetX()[i], inGr->GetY()[i]/peak);
    }

    return og;
  }


TGraph2D * RUtil::normalize(TGraph2D * inGr){
    double length=inGr->GetN();
    double norm=0;
    for(int i=0;i<length;i++){
      norm+=inGr->GetZ()[i]*inGr->GetZ()[i];
    }
    TGraph2D *og = (TGraph2D*)inGr->Clone();
    for(int i=0;i<length;i++)og->GetZ()[i]/=sqrt(norm);
    return og;
}

double RUtil::normalize(TH2D * inGr, double ymin, double ymax){
  //auto oG=(TH2D*)inGr->Clone();
  auto norm=RUtil::integrate(inGr, inGr->GetXaxis()->GetXmin(), inGr->GetXaxis()->GetXmax(),ymin, ymax);
  inGr->Scale(1./norm);
  return norm;
}

double RUtil::norm(TH2D * inGr, double ymin, double ymax){

  auto norm=RUtil::integrate(inGr, inGr->GetXaxis()->GetXmin(), inGr->GetXaxis()->GetXmax(), ymin, ymax);

  return norm;
}


TGraph * RUtil::CDF(TGraph * inGr, int normed){
  auto outGr=new TGraph(inGr->GetN());
    double val=0;
  for(int i=0;i<inGr->GetN();i++){
    outGr->SetPoint(i, inGr->GetX()[i], val);
    val+=inGr->GetY()[i];
  }
  if(normed==1){
    auto ynormGr=RUtil::scale(outGr, 1./outGr->GetY()[outGr->GetN()-1]);
    auto xnormGr=RUtil::stretch(ynormGr, 1./outGr->GetX()[outGr->GetN()-1]);
    outGr=(TGraph*)xnormGr->Clone();
    delete(ynormGr);
    delete(xnormGr);
  }

  return outGr;
}

TGraph * RUtil::absGraph(TGraph * inGr){
  auto outGr=new TGraph(inGr->GetN());
  for(int i=0;i<inGr->GetN();i++){
    outGr->SetPoint(i, inGr->GetX()[i], abs(inGr->GetY()[i]));
  }
  return outGr;
}

int RUtil::absHist(TH2D * inGr){
  for(int i=0;i<inGr->GetNbinsX();i++){
    for(int j=0;j<inGr->GetNbinsY();j++){
      inGr->SetBinContent(i,j, abs(inGr->GetBinContent(i,j)));
    }
  }
  return 1.;
}


// double RUtil::getInstPhase(TGraph *inGr, double t, int deg0rad1){
//   //auto amp=RUtil::rms(inGr, inGr->GetX()[0], inGr->GetX()[inGr->GetN()-1])*sqrt(2);
//   auto norm=RUtil::normToPeak(inGr);
//   //  auto tIndex=RUtil::getIndex(norm, t);
//   auto amp=norm->Eval(t);
//   auto val=asin(amp);
  
//   if(norm->Eval(t+.05)<amp){
//     val=RUtil::pi-val;
//   }
//   if(deg0rad1==0){
//     return val/RUtil::deg;
//   }
//   return val;
// }

int RUtil::getInitPhaseAmp(TGraph *event, double freq, double * amp, double *phase){
  auto omega=RUtil::twoPi*freq;
  auto t1 = event->GetX()[0];
  auto t2 = event->GetX()[1];
  
  auto y1 = event->GetY()[0];
  auto y2 = event->GetY()[1];
  
  auto A = (y2*cos(omega*t1)-y1*cos(omega*t2))/sin(omega*(t2-t1));
  auto B = (y1*sin(omega*t2)-y2*sin(omega*t1))/sin(omega*(t2-t1));
  
  *amp = sqrt(A*A+B*B);
  
  *phase = fmod(atan2(B,A), RUtil::twoPi);
  
  
  return 1;
}

vector<double> RUtil::toVector(int *data, int n){
  vector<double> outD;
  for(int i=0;i<n;i++){
    outD.push_back(data[i]);
  }
  return outD;
}

vector<double> RUtil::toVector(double *data, int n){
  vector<double> outD;
  for(int i=0;i<n;i++){
    outD.push_back(data[i]);
  }
  return outD;
}

vector<double> RUtil::toVector(TH1F * data){
  vector<double> outD;
  for(int i=0;i<data->GetNbinsX();i++){
    outD.push_back(data->GetBinContent(i));
  }
  return outD;
}

vector<double> RUtil::toVector(TH1D * data){
  vector<double> outD;
  for(int i=0;i<data->GetNbinsX();i++){
    outD.push_back(data->GetBinContent(i));
  }
  return outD;
}


double RUtil::mean(TGraph *gr, double t_low, double t_high){
  auto data=gr->GetY();
  int N=gr->GetN();

  t_low=t_low<gr->GetX()[0]?gr->GetX()[0]:t_low;
  t_high=t_high>gr->GetX()[N-1]?gr->GetX()[N-1]:t_high;

  double m=0.;

  double n=0.;
  for(int i=0;i<N;i++){
    if(gr->GetX()[i]>t_low&&gr->GetX()[i]<t_high){
      m+=data[i];
      n+=1.;
    }
  }
  
  return m/n;
}

double RUtil::maxInRange(TGraph *gr, double t_low, double t_high){
  auto data=gr->GetY();
  int N=gr->GetN();

  t_low=t_low<gr->GetX()[0]?gr->GetX()[0]:t_low;
  t_high=t_high>gr->GetX()[N-1]?gr->GetX()[N-1]:t_high;
  auto graph=RUtil::getChunkOfGraph(gr, t_low, t_high);
  auto maxx=TMath::MaxElement(graph->GetN(), graph->GetY());
  delete graph;
  return maxx;
}

double RUtil::max(TGraph *gr){

  auto maxx=TMath::MaxElement(gr->GetN(), gr->GetY());
  
  return maxx;
}
// double RUtil::max(int N, double * dat){

//   auto max=TMath::MaxElement(N, dat);
  
//   return max;
// }


double RUtil::locMax(TGraph *gr){

  auto maxx=gr->GetX()[TMath::LocMax(gr->GetN(), gr->GetY())];
  
  return maxx;
}
double RUtil::locMaxInRange(TGraph *gr, double t_low, double t_high){
  auto data=gr->GetY();
  int N=gr->GetN();

  t_low=t_low<gr->GetX()[0]?gr->GetX()[0]:t_low;
  t_high=t_high>gr->GetX()[N-1]?gr->GetX()[N-1]:t_high;
  auto graph=RUtil::getChunkOfGraph(gr, t_low, t_high);
  auto maxx=graph->GetX()[TMath::LocMax(graph->GetN(), graph->GetY())];
  delete graph;  
  return maxx;
}


double RUtil::minInRange(TGraph *gr, double t_low, double t_high){
  auto data=gr->GetY();
  int N=gr->GetN();

  t_low=t_low<gr->GetX()[0]?gr->GetX()[0]:t_low;
  t_high=t_high>gr->GetX()[N-1]?gr->GetX()[N-1]:t_high;
  auto graph=RUtil::getChunkOfGraph(gr, t_low, t_high);
  auto min=TMath::MinElement(graph->GetN(), graph->GetY());
  delete graph;
  return min;
}

double RUtil::min(TGraph *gr){

  auto min=TMath::MinElement(gr->GetN(), gr->GetY());
  
  return min;
}

double RUtil::locMin(TGraph *gr){

  auto min=gr->GetX()[TMath::LocMin(gr->GetN(), gr->GetY())];
  
  return min;
}
double RUtil::locMinInRange(TGraph *gr, double t_low, double t_high){
  auto data=gr->GetY();
  int N=gr->GetN();

  t_low=t_low<gr->GetX()[0]?gr->GetX()[0]:t_low;
  t_high=t_high>gr->GetX()[N-1]?gr->GetX()[N-1]:t_high;
  auto graph=RUtil::getChunkOfGraph(gr, t_low, t_high);
  auto min=graph->GetX()[TMath::LocMin(graph->GetN(), graph->GetY())];
  delete graph;  
  return min;
}


int RUtil::getIndex(TGraph * gr, double t){
  int index=0;
  for(int i=0;i<gr->GetN()-1;i++){
    if(gr->GetX()[i+1]>t){
      index=i;
      return index;
    }
  }
  return 0;
}

int RUtil::getIndex(TGraph2D * gr, double t){
  int index=0;
  double minVal=999999.;
  for(int i=0;i<gr->GetN()-1;i++){
    if(gr->GetX()[i+1]>t){
      index=i;
      return index;
    }
  }
  // for(int i=0;i<gr->GetN()-1;i++){
  //   auto val=abs(gr->GetX()[i]-t);
  //   if(val<minVal){
  //     minVal=val;
  //     index=i;
  //   }
  //   //if(gr->GetX()[i]==t||gr->GetX()[i+1]>t){
  //   // cout<<gr->GetX()[i]<<" "<<gr->GetX()[i+1]<<endl;
  //   // index=i;
  //   // return index;
  //   //}
  // }
  
  return 0;
}

double RUtil::snr(TGraph *gr, double noiseStart, double noiseEnd){
  double noise=1;
  if(noiseEnd==0.){
    noise=RUtil::rms(gr, 0, RUtil::locMax(gr));
  }
  else{
    noise=RUtil::rms(gr, noiseStart, noiseEnd);
  }
  auto sig=RUtil::max(gr)/sqrt(2);
  return sig/noise;
}


TGraph * RUtil::removeMean(TGraph *gr, double t_low, double t_high){
  double m=RUtil::mean(gr, t_low, t_high);
  return shiftY(gr, -m);
}

int RUtil::removeMeanInPlace(TGraph *gr, double t_low, double t_high){
  double m=RUtil::mean(gr, t_low, t_high);
  for(int i=0;i<gr->GetN();i++){
    gr->GetY()[i]+=(-m);
  }
  return 1;
}

TGraph * RUtil::power(TGraph *gr){
  auto outGr=(TGraph*)gr->Clone();
  for(int i=0;i<outGr->GetN();i++){
    outGr->SetPoint(i, outGr->GetX()[i], outGr->GetY()[i]*outGr->GetY()[i]/50.);
  }
  return outGr;
}

TGraph * RUtil::squared(TGraph *gr){
  auto outGr=(TGraph*)gr->Clone();
  for(int i=0;i<outGr->GetN();i++){
    outGr->SetPoint(i, outGr->GetX()[i], outGr->GetY()[i]*outGr->GetY()[i]);
  }
  return outGr;
}

int RUtil::squareHist(TH2D *inGr){
  for(int i=0;i<inGr->GetNbinsX();i++){
    for(int j=0;j<inGr->GetNbinsY();j++){
      inGr->SetBinContent(i,j, inGr->GetBinContent(i,j)*inGr->GetBinContent(i,j));
    }
  }
  return 1.;

}

double RUtil::sumGraph(TGraph *gr, double t_low, double t_high){
  t_low=t_low>0.?t_low:0.;
  t_high>gr->GetX()[gr->GetN()-1]?gr->GetX()[gr->GetN()-1]:t_high;

  double sum=0.;
  for(int i=0;i<gr->GetN();i++){
    if(gr->GetX()[i]<t_low||gr->GetX()[i]>t_high)continue;
    sum+=gr->GetY()[i];
  }
  return sum;

}

// template <typename T> double RUtil::sum(T * data, int index_low, int index_high){
//   if(index_low>index_high){
//     return 0;
//   }
//   double sum=0;
//   for(int i=index_low;i<index_high;i++){
//     sum+=(double)data[i];
//   }
//   return sum;
// }
double RUtil::integrate(TGraph * gr, double t_low, double t_high){
  t_low=t_low>0.?t_low:0.;
  t_high>gr->GetX()[gr->GetN()-1]?gr->GetX()[gr->GetN()-1]:t_high;
  double dt = gr->GetX()[1]-gr->GetX()[0];
  double integral=0.;
  for(int i=0;i<gr->GetN();i++){
    if(gr->GetX()[i]<t_low||gr->GetX()[i]>t_high)continue;
    integral+=gr->GetY()[i]*dt;
  }
  return integral;
}

TGraph * RUtil::derivative(TGraph *gr, int direction){
  double x, y, lastx, lasty;
  int n=gr->GetN();
  lastx=gr->GetX()[0];
  lasty=gr->GetY()[0];
  
  auto outGr=new TGraph();
  for(int i=1;i<n;i++){
    x=gr->GetX()[i];
    y=gr->GetY()[i];
    
    auto dx=x-lastx;
    auto dy=y-lasty;
    outGr->SetPoint(outGr->GetN(), lastx, direction*dy/dx);
    lastx=x;
    lasty=y;
  }
  return outGr;
}

double RUtil::rms(TGraph * gr, double t_low, double t_high){
  t_low=t_low>0.?t_low:0.;
  t_high=t_high>gr->GetX()[gr->GetN()-1]?gr->GetX()[gr->GetN()-1]:t_high;
  double n=0.;
  double rms=0.;
  for(int i=0;i<gr->GetN();i++){
    if(gr->GetX()[i]>t_low&&gr->GetX()[i]<t_high){
      rms+=(gr->GetY()[i]*gr->GetY()[i]);
      n+=1.;
    }
  }
  return sqrt(rms/n);
}

// double RUtil::rms(int N, double *data){
//   double rms=0.;
//   for(int i=0;i<N;i++){
//     rms+=(data[i]*data[i]);
//   }
//   return sqrt(rms/N);
// }

double RUtil::amplitude(TGraph * gr, double t_low, double t_high){
  t_low=t_low>0.?t_low:0.;
  t_high=t_high>gr->GetX()[gr->GetN()-1]?gr->GetX()[gr->GetN()-1]:t_high;
  double n=0.;
  double rms=0.;
  for(int i=0;i<gr->GetN();i++){
    if(gr->GetX()[i]>=t_low&&gr->GetX()[i]<=t_high){
      rms+=(gr->GetY()[i]*gr->GetY()[i]);
      n+=1.;
    }
  }
  return sqrt(rms/n)*sqrt(2.);
}


TGraph * RUtil::getZeroCrossGraph(TGraph * inGr, int relative){
  auto outGr=new TGraph();
  double val=0.;
  double lastVal=0.;
  double lastX=0.;
  int num=0;
  for(int i=0;i<inGr->GetN();i++){
    val=inGr->GetY()[i];
    if((val>0.&&lastVal<0.)||(val<0.&&lastVal>0.)){
      if(relative==0){
	outGr->SetPoint(outGr->GetN(), num, inGr->GetX()[i]);
      }
      else{
	outGr->SetPoint(outGr->GetN(), num, inGr->GetX()[i]-lastX);
	lastX=inGr->GetX()[i];
      }
      num++;
    }
    lastVal=val;

  }
  return outGr;
}

TH1F * RUtil::histogram(TGraph *gr, int nbins, TString title, TString name){
  //auto len=sizeof(vals)/sizeof(vals[0]);
  auto vals=gr->GetY();
  auto len=gr->GetN();
  auto xlow=RUtil::min(gr);
  auto xhigh=RUtil::max(gr);
  auto hh=new TH1F(name, title, nbins, xlow, xhigh);
  for(int i=0;i<len;i++){
    hh->Fill(vals[i]);
  }
  return hh;
}

TH1D * RUtil::toHistogram(TGraph *gr, int nbins, TString title, TString name){
  //auto len=sizeof(vals)/sizeof(vals[0]);
  auto vals=gr->GetY();
  auto len=gr->GetN();
  auto xlow=gr->GetX()[0];
  auto xhigh=gr->GetX()[gr->GetN()-1];
  auto hh=new TH1D(name, title, nbins, xlow, xhigh);
  for(int i=0;i<nbins;i++){
    hh->SetBinContent(i, gr->Eval(hh->GetBinLowEdge(i)));
  }
  return hh;
}

int RUtil::fillZeroCrossHist(TGraph * inGr, TH1D* hist, double weight, double threshold){
  double val=0., nextVal=0.;
  double lastVal=0.;
  double lastX=0.;
  int num=0;
  for(int i=0;i<inGr->GetN()-1;i++){
    val=inGr->GetY()[i];
    nextVal=inGr->GetY()[i+1];
    if((val>threshold&&nextVal<-threshold)||(val<-threshold&&nextVal>threshold)){
      hist->Fill(inGr->GetX()[i], weight);
      num++;
    }

    //    lastVal=val;

  }
  return num;
}

int RUtil::assignXOffset(TGraph *inGr, double * offsets, double constant){
  for(int i=0;i<inGr->GetN();i++){
    if(offsets[i]==0)continue;
    inGr->SetPoint(i, inGr->GetX()[i]+constant*offsets[i], inGr->GetY()[i]);
  }
  return 1;
}

TGraph * RUtil::gObs(TGraph *inGr, double thetaDeg, double tUnits){
  auto tRet=inGr->GetX();
  auto aRet=inGr->GetY();
  auto N=inGr->GetN();
  auto cc=RUtil::c_light*tUnits;//adjust the speed of light for the observer time (if needed)
  auto n=1.0003;
  auto theta=RUtil::deg2Rad(thetaDeg);
  vector<double>tObs;
  vector<double>aObs;
  
  for(int i=0;i<N;i++){
    double R=cc*tRet[i]/cos(theta);//for an observer on the ground theta degrees from shower axis

    tObs.push_back((R/cc)+tRet[i]);
    double D=(R*(1.-n*cos(theta)));

    auto A=aRet[i]/abs(D);
    if(R==0)A=0;
    aObs.push_back(A);
  }
  auto outGr=new TGraph(tObs.size(), &tObs[0], &aObs[0]);

  return outGr;
}



double RUtil::integratePower(TGraph * gr, double t_low, double t_high){
  t_low=t_low>0.?t_low:0.;
  t_high>gr->GetX()[gr->GetN()-1]?gr->GetX()[gr->GetN()-1]:t_high;
  double dt = gr->GetX()[1]-gr->GetX()[0];
  double integral=0.;
  for(int i=0;i<gr->GetN();i++){
    if(gr->GetX()[i]<t_low||gr->GetX()[i]>t_high)continue;
    integral+=(gr->GetY()[i]*gr->GetY()[i])*dt;
  }
  return integral;
}

double RUtil::avgPower(TGraph * gr, double t_low, double t_high){
  t_low=t_low>0.?t_low:0.;
  t_high>gr->GetX()[gr->GetN()-1]?gr->GetX()[gr->GetN()-1]:t_high;
  double dt = gr->GetX()[1]-gr->GetX()[0];
  double integral=0.;
  double count=0.;
  for(int i=0;i<gr->GetN();i++){
    if(gr->GetX()[i]<t_low||gr->GetX()[i]>t_high)continue;
    integral+=(gr->GetY()[i]*gr->GetY()[i]/50.);
    count+=1.;
  }
  return integral/count;
}



// double * RUtil::flip(int n, double * in){
//   double out[n];
//   for(int i=0;i<n;i++){
//     out[n]=in[n-1-i];
//   }
//   return out;
// }

TGraph * RUtil::flip(TGraph *inGr){
  auto outGr=new TGraph();
  for(int i=0;i<inGr->GetN();i++){
    outGr->SetPoint(outGr->GetN(), inGr->GetX()[i], inGr->GetY()[inGr->GetN()-1-i]);
  }
  return outGr;
}

TGraph * RUtil::swap(TGraph *inGr){
  auto outGr=new TGraph(inGr->GetN(), inGr->GetY(), inGr->GetX());
    return outGr;
}

TGraph * RUtil::integrateByBin(TGraph *gr, double binNS){
  int nbins=gr->GetX()[gr->GetN()-1]/binNS;
  auto outGr=new TGraph(nbins);

  double t=binNS;
  for(int i=0;i<nbins;i++){
    outGr->SetPoint(i, t, integrate(gr, t-binNS, t));
    t+=binNS;
  }

  return outGr;

}

TGraph * RUtil::avgGraph(vector<TGraph*> inGr){
  auto outGr=new TGraph(inGr[0]->GetN());
  for(int i=0;i<inGr.size();i++){
    for(int j=0;j<outGr->GetN();j++){
      outGr->SetPoint(j, inGr[0]->GetX()[j], inGr[i]->GetY()[j]+outGr->GetY()[j]);
    }
  }
    return RUtil::scale(outGr, 1./inGr.size());
}

TGraph * RUtil::add(TGraph *g1, TGraph *g2, double constant){

  int len=g1->GetN()<g2->GetN()?g1->GetN():g2->GetN();
  TGraph *outGr=new TGraph(len);  
  for(int i=0;i<len;i++){
    outGr->SetPoint(i, g1->GetX()[i], g1->GetY()[i]+(constant*g2->Eval(g1->GetX()[i])));
  }
  outGr->GetXaxis()->SetTitle(g1->GetXaxis()->GetTitle());
  outGr->GetYaxis()->SetTitle(g1->GetYaxis()->GetTitle());

  return outGr;
}

int RUtil::add(double *result, int N, double *x, double *y, double constant){

  int len=N;

  for(int i=0;i<len;i++){
    result[i]=x[i]+(y[i]*constant);
  }
  return 1;
}

TGraph2D * RUtil::add(TGraph2D *g1, TGraph2D *g2, double constant){

  int len=g1->GetN()<g2->GetN()?g1->GetN():g2->GetN();
  TGraph2D *outGr=new TGraph2D(len);  
  for(int i=0;i<len;i++){
    outGr->SetPoint(i, g1->GetX()[i], g1->GetY()[i], g1->GetZ()[i]+(g2->GetZ()[i]*constant));
  }
  outGr->GetXaxis()->SetTitle(g1->GetXaxis()->GetTitle());
  outGr->GetYaxis()->SetTitle(g1->GetYaxis()->GetTitle());

  return outGr;
}

TGraph * RUtil::fmodGraph(TGraph *g, double mod){
  auto y=g->GetY();
  TGraph *outGr=new TGraph(g->GetN());  
  for(int i=0;i<g->GetN();i++){
    outGr->SetPoint(i, g->GetX()[i], fmod(g->GetY()[i], mod));
  }
  return outGr;
}

double RUtil::dot(TGraph *g1, TGraph *g2){
  auto x = g1->GetY();
  auto y = g2->GetY();
  double num=0., xdenom=0., ydenom=0.;
  auto n=g1->GetN()<g2->GetN()?g1->GetN():g2->GetN();
  for(int i=0;i<n;i++){
    num+=(x[i])*(y[i]);
    xdenom+=pow(x[i], 2);
    ydenom+=pow(y[i], 2);
  }
  return num/sqrt(xdenom*ydenom);
  
}

double RUtil::dot(double * x, double *y, int N){
  auto n=N;
  double num=0., xdenom=0., ydenom=0.;
  for(int i=0;i<n;i++){
    num+=(x[i])*(y[i]);
    xdenom+=pow(x[i], 2);
    ydenom+=pow(y[i], 2);
  }
  return num/sqrt(xdenom*ydenom);
  
}

double RUtil::dot(TGraph *g1, TGraph *g2, double tLow, double tHigh){
  auto x = g1->GetY();
  auto y = g2->GetY();
  double num=0., xdenom=0., ydenom=0.;
  auto n=g1->GetN()<g2->GetN()?g1->GetN():g2->GetN();
  for(int i=0;i<n;i++){
    if(g1->GetX()[i]>tLow&&g1->GetX()[i]<tHigh){
      num+=(x[i])*(y[i]);
      xdenom+=pow(x[i], 2);
      ydenom+=pow(y[i], 2);
    }
  }
  return num/sqrt(xdenom*ydenom);
  
}


TGraph * RUtil::mult(TGraph *g1, TGraph *g2, double constant){

  int len=g1->GetN()<g2->GetN()?g1->GetN():g2->GetN();
  TGraph *outGr=new TGraph(len);  
  for(int i=0;i<len;i++){
    outGr->SetPoint(i, g1->GetX()[i], g1->GetY()[i]*(constant*g2->Eval(g1->GetX()[i])));
  }
  outGr->GetXaxis()->SetTitle(g1->GetXaxis()->GetTitle());
  outGr->GetYaxis()->SetTitle(g1->GetYaxis()->GetTitle());

  return outGr;
}

TGraph2D * RUtil::multComplex(TGraph2D *g1, TGraph2D *g2){

  int len=g1->GetN()<=g2->GetN()?g1->GetN():g2->GetN();
  TGraph2D *outGr=new TGraph2D(len);  
  for(int i=0;i<len;i++){
    auto x1=g1->GetX()[i];
    auto y1=g1->GetY()[i];
    auto z1=g1->GetZ()[i];
    auto y2=g2->GetY()[i];
    auto z2=g2->GetZ()[i];
    auto y3=y1*y2 - z1*z2;
    auto z3=y1*z2 + z1*y2;
    outGr->SetPoint(i, x1, y3, z3);
  }
  outGr->GetXaxis()->SetTitle(g1->GetXaxis()->GetTitle());
  outGr->GetYaxis()->SetTitle(g1->GetYaxis()->GetTitle());

  return outGr;
}


TGraph * RUtil::divide(TGraph *g1, TGraph *g2, double constant){

  int len=g1->GetN()<g2->GetN()?g1->GetN():g2->GetN();
  TGraph *outGr=new TGraph(len);  
  for(int i=0;i<len;i++){
    auto val=g1->GetY()[i]/(constant*g2->Eval(g1->GetX()[i]));
  if(isfinite(val)){
    outGr->SetPoint(i, g1->GetX()[i], val);
  }
  else{
    outGr->SetPoint(i, g1->GetX()[i], 0);
  }
  }
  outGr->GetXaxis()->SetTitle(g1->GetXaxis()->GetTitle());
  outGr->GetYaxis()->SetTitle(g1->GetYaxis()->GetTitle());

  return outGr;
}

vector<double> RUtil::divide(vector<double> v1, vector<double> v2, double constant){

  int len=v1.size()<v2.size()?v1.size():v2.size();
  vector<double> outV;
  for(int i=0;i<len;i++){
    auto val=v1[i]/(constant*v2[i]);
  if(isfinite(val)){
    outV.push_back(val);
  }
  else{
    outV.push_back(0);
  }
  }

  return outV;
}


TGraph * RUtil::scale(TGraph *g1, double factor){
  TGraph *outGr=new TGraph(g1->GetN());
  for(int i=0;i<g1->GetN();i++){
    outGr->SetPoint(i, g1->GetX()[i], g1->GetY()[i]*factor);
  }
  outGr->SetName(g1->GetName());
  outGr->SetTitle(g1->GetTitle());
  outGr->GetXaxis()->SetTitle(g1->GetXaxis()->GetTitle());
  outGr->GetYaxis()->SetTitle(g1->GetYaxis()->GetTitle());

  return outGr;
  
}

int RUtil::scale(double * result, int N, double *input, double factor){
  for(int i=0;i<N;i++){
    result[i]=input[i]*factor;

  }
  return 1;
  
}

TGraph * RUtil::stretch(TGraph *g1, double factor){
  TGraph *outGr=new TGraph(g1->GetN());
  for(int i=0;i<g1->GetN();i++){
    outGr->SetPoint(i, g1->GetX()[i]*factor, g1->GetY()[i]);
  }
  outGr->SetName(g1->GetName());
  outGr->SetTitle(g1->GetTitle());
  outGr->GetXaxis()->SetTitle(g1->GetXaxis()->GetTitle());
  outGr->GetYaxis()->SetTitle(g1->GetYaxis()->GetTitle());

  return outGr;

}


TGraph * RUtil::shiftY(TGraph *g1, double factor){
  TGraph *outGr=new TGraph(g1->GetN());
  for(int i=0;i<g1->GetN();i++){
    outGr->SetPoint(i, g1->GetX()[i], g1->GetY()[i]+factor);
  }
  outGr->SetName(g1->GetName());
  outGr->SetTitle(g1->GetTitle());
  outGr->GetXaxis()->SetTitle(g1->GetXaxis()->GetTitle());
  outGr->GetYaxis()->SetTitle(g1->GetYaxis()->GetTitle());

  return outGr;
  
}

TGraphErrors * RUtil::shiftY(TProfile *g1, double factor){
  TGraphErrors *outGr=new TGraphErrors();
  g1->SetErrorOption("i");
  for(int i=0;i<g1->GetNbinsX();i++){
    if(g1->GetBinContent(i)!=0){
      outGr->SetPoint(outGr->GetN(), g1->GetBinCenter(i), g1->GetBinContent(i)+factor);
      outGr->SetPointError(outGr->GetN()-1, (g1->GetBinCenter(1)-g1->GetBinCenter(0))/2.,g1->GetBinError(i) );
    }
  }
  //  outGr->SetName(g1->GetName());
  //outGr->SetTitle(g1->GetTitle());
  outGr->GetXaxis()->SetTitle(g1->GetXaxis()->GetTitle());
  outGr->GetYaxis()->SetTitle(g1->GetYaxis()->GetTitle());

  return outGr;
  
}

TGraph * RUtil::shiftX(TGraph *g1, double factor){
  TGraph *outGr=new TGraph(g1->GetN());
  for(int i=0;i<g1->GetN();i++){
    outGr->SetPoint(i, g1->GetX()[i]+factor, g1->GetY()[i]);
  }
  outGr->SetName(g1->GetName());
  outGr->SetTitle(g1->GetTitle());
  outGr->GetXaxis()->SetTitle(g1->GetXaxis()->GetTitle());
  outGr->GetYaxis()->SetTitle(g1->GetYaxis()->GetTitle());

  return outGr;
  
}

vector<pair<int, double>> RUtil::sort(vector<double> inVec, int order){
  auto indexData=vector<std::pair<int, double>>();
  for(int i=0;i<inVec.size();i++){
    std::pair <int, double> var(i, inVec[i]);
    indexData.push_back(var);
}

  std::sort(indexData.begin(), indexData.end(),[](const pair<int, double> &a, const pair<int, double> &b) { return a.second < b.second; });
  if(order==1){
    std::reverse(indexData.begin(), indexData.end());
  }
  return indexData;
}

double RUtil::peakiness(TGraph *inGr){
  auto mmax=RUtil::max(inGr);
  auto locmmax=RUtil::locMax(inGr);
  auto dt=inGr->GetX()[1]-inGr->GetX()[0];
  auto meanPre=RUtil::mean(inGr, 0, locmmax-dt);
  auto meanPost=RUtil::mean(inGr, locmmax+dt, inGr->GetX()[inGr->GetN()-1]);
  return mmax/((meanPre+meanPost)/2.);
}
// RUtil::dedisperse(TGraph2D * one, TGraph2D * two){
//   auto re1=

double RUtil::getFirstThresholdCrossing(TGraph *inGr, double thresh, double after, int rising){
  double thisVal;
  double lastVal;
  double crossTime=0.;
  for(int i=0;i<inGr->GetN();i++){
    if(inGr->GetX()[i]<after)continue;
    thisVal=inGr->GetY()[i];
    if(rising==1){
      if(thisVal>thresh&&lastVal<=thresh){
	crossTime=inGr->GetX()[i];
	break;
      }
    }
    else{
      if(thisVal<thresh&&lastVal>=thresh){
        crossTime=inGr->GetX()[i];
        break;
      }
    }
  }
    return crossTime;
}

double RUtil::getLastThresholdCrossing(TGraph *inGr, double thresh, double after, int rising){
  double thisVal;
  double lastVal;
  double crossTime=0.;
  for(int i=0;i<inGr->GetN();i++){
    if(inGr->GetX()[i]<after)continue;
    thisVal=inGr->GetY()[i];
    if(rising==1){
      if(thisVal>thresh&&lastVal<=thresh){
	crossTime=inGr->GetX()[i];
	//break;
      }
    }
    else{
      if(thisVal<thresh&&lastVal>=thresh){
        crossTime=inGr->GetX()[i];
	// break;
      }
    }

  }
    return crossTime;
}

double RUtil::sinc(double x){
  if(x==0.){
    return 1.;
  }
  return sin(pi* x)/(pi*x);
}




TGraph * RUtil::sincInterpolateGraph(TGraph *inGr, double interpGSs){
  double T = inGr->GetX()[1]-inGr->GetX()[0];
  double dt = 1./interpGSs;
  vector<double> xx, yy;
  double t=0;
  double num=0.;
  
  t=0;    
  while(t<inGr->GetX()[inGr->GetN()-1]){
    xx.push_back(t);
    double temp=0;
    for(int i=0;i<inGr->GetN();i++){
      temp+=inGr->GetY()[i]*RUtil::sinc((t-((double)i*T))/T);

    }
  
    yy.push_back(temp);
    t+=dt;
  }
  TGraph *outGr = new TGraph(xx.size(), &xx[0], &yy[0]);
  return outGr;
}
//experimental
TGraph * RUtil::sincInterpolateGraphDev(TGraph *inGr, double factor, int N){
  double dataT=inGr->GetX()[1]-inGr->GetX()[0];
  double targetT=dataT/factor;
  double interpGSs=1./targetT;
  // double dt = 1./interpGSs;
  // vector<double> xx, yy;
  // double t=0;
  // double num=0.;
  
  // t=0;    
  // while(t<inGr->GetX()[inGr->GetN()-2]){
  //   xx.push_back(t);
  //   double temp=0;
  //   for(int i=0;i<inGr->GetN()-1;i++){
  //     double T = inGr->GetX()[i+1]-inGr->GetX()[i];
  //     temp+=inGr->GetY()[i]*RUtil::sinc((t-((double)i*T))/T);

  //   }
  
  //   yy.push_back(temp);
  //   t+=dt;
  // }
  // TGraph *outGr = new TGraph(xx.size(), &xx[0], &yy[0]);
  // return outGr;
  return sincInterpolateGraphFast(inGr, interpGSs, N);
}


TGraph * RUtil::sincInterpolateGraphFast(TGraph *inGr, double interpGSs, int N){
  double T = inGr->GetX()[1]-inGr->GetX()[0];
  double dt = 1./interpGSs;
  vector<double> xx, yy;
  double t=0;
  double num=0.;
  
  t=0;    
  while(t<inGr->GetX()[inGr->GetN()-1]){
    int k=t/T;
    xx.push_back(t);
    double temp=0;
    for(int i=k-N;i<k+N;i++){
    if(i<0||i>=inGr->GetN())continue;
      temp+=inGr->GetY()[i]*RUtil::sinc((t-((double)i*T))/T);

    }
  
    yy.push_back(temp);
    t+=dt;
  }
  TGraph *outGr = new TGraph(xx.size(), &xx[0], &yy[0]);
  return outGr;
}




int RUtil::getInterpolatedGraph(TGraph * inGraph, TGraph *outGraph, double interpGSs, int type, int N){
  if(type==0){
    auto grTemp=interpolateGraph(inGraph, interpGSs);
    *outGraph=*grTemp;
    delete(grTemp);
  }
  else if (type==1){
    auto grTemp=sincInterpolateGraph(inGraph, interpGSs);
    *outGraph=*grTemp;
    delete(grTemp);
  }
  else if (type==2){
    auto grTemp=sincInterpolateGraphFast(inGraph, interpGSs, N);
    *outGraph=*grTemp;
    delete(grTemp);
  }

  return 1;
}




TGraph * RUtil::interpolateGraph(TGraph * inGraph, double interpGSs){
  ROOT::Math::Interpolator interp(inGraph->GetN(), ROOT::Math::Interpolation::kAKIMA);
  interp.SetData(inGraph->GetN(), inGraph->GetX(), inGraph->GetY());

  //get dt, assuming even sampling.
  double inDt=inGraph->GetX()[50]-inGraph->GetX()[49];
  double inGSs=1./inDt;

  double outDt=1./interpGSs;

  int samps=(int) (inGraph->GetN()*(interpGSs/inGSs));

  vector<double> xx, yy;
  for(int i=0;i<samps;i++){
    double time = i*outDt;
    if(time>inGraph->GetX()[inGraph->GetN()-1])continue;
    xx.push_back(time);
    yy.push_back(interp.Eval(time));
  }

  auto outGr=new TGraph(xx.size(), &xx[0], &yy[0]);

			 

  return outGr;
}


TGraph * RUtil::interpolateGraph(TGraph * inGraph, vector<double> times){
  ROOT::Math::Interpolator interp(inGraph->GetN(), ROOT::Math::Interpolation::kAKIMA);
  interp.SetData(inGraph->GetN(), inGraph->GetX(), inGraph->GetY());

  //get dt, assuming even sampling.

  vector<double> xx, yy;
  for(int i=0;i<times.size();i++){
    xx.push_back(times[i]);
    yy.push_back(interp.Eval(times[i]));
  }

  auto outGr=new TGraph(xx.size(), &xx[0], &yy[0]);

			 

  return outGr;
}

TGraph * RUtil::interpolateGraph(TGraph * inGraph, int N, double* times){
  ROOT::Math::Interpolator interp(inGraph->GetN(), ROOT::Math::Interpolation::kAKIMA);
  interp.SetData(inGraph->GetN(), inGraph->GetX(), inGraph->GetY());

  //get dt, assuming even sampling.

  vector<double> xx, yy;
  for(int i=0;i<N-1;i++){
    xx.push_back(times[i]);
    yy.push_back(interp.Eval(times[i]));
  }

  auto outGr=new TGraph(xx.size(), &xx[0], &yy[0]);

			 

  return outGr;
}




TGraph * RUtil::getChunkOfGraph(TGraph *ingr, double start, double end, int delay_to_zero){
  ingr->SetBit(TGraph::kIsSortedX);
  double *xx=ingr->GetX();
  double *yy=ingr->GetY();
  vector<double> outx, outy;
  double xincr=xx[10]-xx[9];
  for(int i=0;i<ingr->GetN();i++){
    //if(xx[i]>=start&&xx[i]<=end){
      //    }
      //else{
      double time=start+((double)i*xincr);
      if(time<end){
      outx.push_back(time);
      //      outx.push_back(xx[i]);
      outy.push_back(ingr->Eval(time));
      }
  }

  TGraph * outg=new TGraph(outx.size(), &outx[0], &outy[0]);
  //outg->SetTitle(ingr->GetTitle());
  outg->SetName("chunk");
  outg->GetXaxis()->SetTitle(ingr->GetXaxis()->GetTitle());
  outg->GetYaxis()->SetTitle(ingr->GetYaxis()->GetTitle());
  if(delay_to_zero==0){
    return outg;
  }
  TGraph *outg2=new TGraph();
  delayGraph(outg, outg2, -start);
  delete outg;
  return outg2;

}

TGraph * RUtil::getChunkOfGraphFast(TGraph *ingr, double start, double end, int delay_to_zero){
  double *xx=ingr->GetX();
  double *yy=ingr->GetY();
  vector<double> outx, outy;
  
  for(int i=0;i<ingr->GetN();i++){
    if(xx[i]>=start&&xx[i]<=end){
      outx.push_back(xx[i]);
      outy.push_back(yy[i]);
    }
  }

  TGraph * outg=new TGraph(outx.size(), &outx[0], &outy[0]);
  outg->SetTitle(ingr->GetTitle());
  outg->SetName(ingr->GetName());
  outg->GetXaxis()->SetTitle(ingr->GetXaxis()->GetTitle());
  outg->GetYaxis()->SetTitle(ingr->GetYaxis()->GetTitle());
  if(delay_to_zero==0){
    return outg;
  }
  return delayGraph(outg, -start);

}

TGraph * RUtil::getNSamplesFrom(TGraph *ingr, double start, int nSamples, int delay_to_zero){
  double *xx=ingr->GetX();
  double *yy=ingr->GetY();
  vector<double> outx, outy;
  int nSamp=0;
  for(int i=0;i<ingr->GetN();i++){
    if(xx[i]>=start&&nSamp<nSamples){
      outx.push_back(xx[i]);
      outy.push_back(yy[i]);
      nSamp++;
    }
  }

  TGraph * outg=new TGraph(outx.size(), &outx[0], &outy[0]);
  outg->SetTitle(ingr->GetTitle());
  outg->SetName(ingr->GetName());
  outg->GetXaxis()->SetTitle(ingr->GetXaxis()->GetTitle());
  outg->GetYaxis()->SetTitle(ingr->GetYaxis()->GetTitle());
  if(delay_to_zero==0){
    return outg;
  }
  auto outgdelayed=new TGraph();
  delayGraph(outg,outgdelayed, -start);
  delete outg;
  return outgdelayed;

}

TGraph * RUtil::getTheseSamples(TGraph *ingr, int sampStart, int sampEnd, int delay_to_zero){
  double *xx=ingr->GetX();
  double *yy=ingr->GetY();
  vector<double> outx, outy;
    for(int i=sampStart;i<sampEnd;i++){
    outx.push_back(xx[i]);
    outy.push_back(yy[i]);
  }

  TGraph * outg=new TGraph(outx.size(), &outx[0], &outy[0]);
  outg->SetTitle(ingr->GetTitle());
  outg->SetName(ingr->GetName());
  outg->GetXaxis()->SetTitle(ingr->GetXaxis()->GetTitle());
  outg->GetYaxis()->SetTitle(ingr->GetYaxis()->GetTitle());
  if(delay_to_zero==0){
    return outg;
  }
  auto outgdelayed=new TGraph();
  delayGraph(outg,outgdelayed, -outx[0]);
  delete outg;
  return outgdelayed;

}


TGraph * RUtil::delayGraph(TGraph *ingr, double delay){
  double*xx=ingr->GetX();
  double xxout[ingr->GetN()];
  double*yy=ingr->GetY();
  for(int i=0;i<ingr->GetN();i++){
    xxout[i]=(double)xx[i]+delay;
  }
  TGraph *dg=new TGraph(ingr->GetN(), xxout, yy);
  dg->SetTitle(ingr->GetTitle());
  dg->SetName(ingr->GetName());
  dg->GetXaxis()->SetTitle(ingr->GetXaxis()->GetTitle());
  dg->GetYaxis()->SetTitle(ingr->GetYaxis()->GetTitle());
  //  delete ingr;
  return dg;
}


int RUtil::delayGraph(TGraph *ingr, TGraph *outgr, double delay){
  double*xx=ingr->GetX();
  double xxout[ingr->GetN()];
  double*yy=ingr->GetY();
  for(int i=0;i<ingr->GetN();i++){
    xxout[i]=(double)xx[i]+delay;
  }
  
  TGraph *dg=new TGraph(ingr->GetN(), xxout, yy);
  //dg->SetTitle(ingr->GetTitle());
  //dg->SetName(ingr->GetName());
  //dg->GetXaxis()->SetTitle(ingr->GetXaxis()->GetTitle());
  //dg->GetYaxis()->SetTitle(ingr->GetYaxis()->GetTitle());

  *outgr=*dg;
  delete dg;
  return 1;
}

int RUtil::roll(double * result, double * vals, int N, int rollN){
  if(rollN>=N)rollN=rollN-N;
  if(rollN<0)rollN=N+rollN;
  for(int i=0;i<N;i++){
    int ind=i+rollN;
    if(ind>=N){
      ind=ind-N;
    }
    if(ind<0){
      ind=N+ind;
    }
    result[i]=vals[ind];

  }

  return rollN;
}

TGraph* RUtil::roll(TGraph * inGr, int rollN){
  double vals[inGr->GetN()];
  RUtil::roll(vals, inGr->GetY(), inGr->GetN(), rollN);
  auto outGr=new TGraph(inGr->GetN(), inGr->GetX(), vals);
  return outGr;
}

double RUtil::wrap(double inval, double low, double high){
  double val=inval;
  if(val<low){
    val=high-(low-val);
  }
  if(val>high){
    val=low+(val-high);
  }
  return val;
 
}
TGraph * RUtil::wrap(TGraph *g, double low, double high){
  auto outGr=new TGraph(g->GetN());
  for(int i=0;i<g->GetN();i++){
    double point=g->GetY()[i];

    outGr->SetPoint(i, g->GetX()[i], RUtil::wrap(point, low, high));
  }
  return outGr;
}

TH1F * RUtil::plotResiduals(TGraph *gr1, TGraph *gr2, int nbins, double min, double maxx){
  TH1F *hist =new TH1F("", "", nbins,min, maxx);
  for(int i=0;i<gr1->GetN();i++)hist->Fill(gr1->GetY()[i]-gr2->GetY()[i]);
  hist->SetLineWidth(3);
  return hist;
}


TGraph * RUtil::crossCorrelate(TGraph * gr1, TGraph * gr2, double max_delay, double t_low, double t_high){
  double *x = gr1->GetY();
  double *time=gr1->GetX();
  double *y = gr2->GetY();
  int yn=gr1->GetN();
  int xn=gr2->GetN();

  int lengthx=xn;
  int lengthy=yn;
  int length=0;
  vector<double> out, outx, outy;
  double num, ynum, xdenom, ydenom, denom;
  double timescale = time[1]-time[0];

  length=lengthx<=lengthy?xn:yn;
  length=lengthx<=lengthy?xn:yn;
  double throwaway=time[xn-1]/10.;//throw away highest delays, they are unstable
  max_delay=max_delay>time[xn-1]?time[xn-1]-throwaway:max_delay;


  t_high=t_high>=time[xn-1]?time[xn-1]:t_high;
  t_low=t_low<time[0]?time[0]:t_low;

  int max_delay_index=(1./timescale)*max_delay;
  double mx=0;
  double my=0;

  int n=0;
  double t=-max_delay;
  for(int n=-max_delay_index;n<max_delay_index;n++){
    //if(time[d]>=-max_delay&&time[d]<=max_delay){
      num=0.;
      xdenom=0.;
      ydenom=0.;
      for(int i=0;i<length;i++){
	if((i+n)>0 && (i+n)<length && time[i]>=t_low && time[i]<=t_high){
	  
	  num+=(x[i]-mx)*(y[i+n]-my);
	  xdenom+=pow(x[i]-mx, 2);
	  ydenom+=pow(y[i+n]-my, 2);
	  
	}
      }
      out.push_back(num/sqrt(xdenom*ydenom));
      //      outx.push_back(time[(length/2)+n]);
      outx.push_back((double)n *timescale);
      //    n++;    
    }



  
  TGraph *outt = new TGraph(outx.size(), &outx[0], &out[0]);
  outt->GetXaxis()->SetTitle("offset (ns)");
  outt->GetYaxis()->SetTitle("CC coefficient");
  outt->GetYaxis()->SetTitleOffset(1.15);
  outt->SetTitle("");
  outt->GetXaxis()->SetRangeUser(-max_delay, max_delay);
  return outt;
}


TGraph * RUtil::crossCorrelateWindowed(TGraph * gr1, TGraph * gr2, TGraph *grWindow, double max_delay, double t_low, double t_high){
  double *x = gr1->GetY();
  double *time=gr1->GetX();
  double *y = gr2->GetY();
  double *window = grWindow->GetY();
  int yn=gr1->GetN();
  int xn=gr2->GetN();

  int lengthx=xn;
  int lengthy=yn;
  int length=0;
  vector<double> out, outx, outy;
  double num, ynum, xdenom, ydenom, denom;
  double timescale = time[1]-time[0];

  length=lengthx<=lengthy?xn:yn;
  length=lengthx<=lengthy?xn:yn;
  double throwaway=time[xn-1]/10.;//throw away highest delays, they are unstable
  max_delay=max_delay>time[xn-1]?time[xn-1]-throwaway:max_delay;

  t_high=t_high>=time[xn-1]?time[xn-1]:t_high;
  t_low=t_low<0.?0.:t_low;

  int max_delay_index=(1./timescale)*max_delay;
  double mx=0;
  double my=0;

  int n=0;
  double t=-max_delay;
  for(int n=-max_delay_index;n<max_delay_index;n++){
    //if(time[d]>=-max_delay&&time[d]<=max_delay){
      num=0.;
      xdenom=0.;
      ydenom=0.;
      for(int i=0;i<length;i++){
	if((i+n)>0 && (i+n)<length && time[i]>=t_low && time[i]<=t_high){
	  
	  num+=(x[i]-mx)*(y[i+n]-my);
	  xdenom+=pow(x[i]-mx, 2);
	  ydenom+=pow(y[i+n]-my, 2);
	  
	}
      }
      out.push_back(num/sqrt(xdenom*ydenom));
      outx.push_back(time[(length/2)+n]);
      //    n++;    
    }


  
  TGraph *outt = new TGraph(outx.size(), &outx[0], &out[0]);

  return outt;
}

TGraph * RUtil::crossAdd(TGraph * gr1, TGraph * gr2, double max_delay, double t_low, double t_high){
  double *x = gr1->GetY();
  double *time=gr1->GetX();
  double *y = gr2->GetY();
  int yn=gr1->GetN();
  int xn=gr2->GetN();

  int lengthx=xn;
  int lengthy=yn;
  int length=0;
  vector<double> out, outx, outy;
  double num, ynum, xdenom, ydenom, denom;
  double timescale = time[1]-time[0];

  length=lengthx<=lengthy?xn:yn;
  length=lengthx<=lengthy?xn:yn;
  double throwaway=time[xn-1]/10.;//throw away highest delays, they are unstable
  max_delay=max_delay>time[xn-1]?time[xn-1]-throwaway:max_delay;


  t_high=t_high>=time[xn-1]?time[xn-1]:t_high;
  t_low=t_low<time[0]?time[0]:t_low;

  int max_delay_index=(1./timescale)*max_delay;
  double mx=0;
  double my=0;

  int n=0;
  double t=-max_delay;
  for(int n=-max_delay_index;n<max_delay_index;n++){
    //if(time[d]>=-max_delay&&time[d]<=max_delay){
      num=0.;
      xdenom=0.;
      ydenom=0.;
      for(int i=0;i<length;i++){
	if((i+n)>0 && (i+n)<length && time[i]>=t_low && time[i]<=t_high){
	  
	  num+=abs(x[i]-mx)+abs(y[i+n]-my);
	  xdenom+=pow(x[i]-mx, 2);
	  ydenom+=pow(y[i+n]-my, 2);
	  
	}
      }
      out.push_back(num/sqrt(xdenom+ydenom));
      //      outx.push_back(time[(length/2)+n]);
      outx.push_back((double)n *timescale);
      //    n++;    
    }



  
  TGraph *outt = new TGraph(outx.size(), &outx[0], &out[0]);
  outt->GetXaxis()->SetTitle("offset (ns)");
  outt->GetYaxis()->SetTitle("CC coefficient");
  outt->GetYaxis()->SetTitleOffset(1.15);
  outt->SetTitle("");
  outt->GetXaxis()->SetRangeUser(-max_delay, max_delay);
  return outt;
}

TGraph * RUtil::crossSubtract(TGraph * gr1, TGraph * gr2, double max_delay, double t_low, double t_high){
  double *x = gr1->GetY();
  double *time=gr1->GetX();
  double *y = gr2->GetY();
  int yn=gr1->GetN();
  int xn=gr2->GetN();

  int lengthx=xn;
  int lengthy=yn;
  int length=0;
  vector<double> out, outx, outy;
  double num, ynum, xdenom, ydenom, denom;
  double timescale = time[1]-time[0];

  length=lengthx<=lengthy?xn:yn;
  length=lengthx<=lengthy?xn:yn;
  double throwaway=time[xn-1]/10.;//throw away highest delays, they are unstable
  max_delay=max_delay>time[xn-1]?time[xn-1]-throwaway:max_delay;


  t_high=t_high>=time[xn-1]?time[xn-1]:t_high;
  t_low=t_low<time[0]?time[0]:t_low;

  int max_delay_index=(1./timescale)*max_delay;
  double mx=0;
  double my=0;

  int n=0;
  double t=-max_delay;
  for(int n=-max_delay_index;n<max_delay_index;n++){
    //if(time[d]>=-max_delay&&time[d]<=max_delay){
      num=0.;
      xdenom=0.;
      ydenom=0.;
      for(int i=0;i<length;i++){
	if((i+n)>0 && (i+n)<length && time[i]>=t_low && time[i]<=t_high){
	  
	  num+=(x[i]-mx)-(y[i+n]-my);
	  xdenom+=pow(x[i]-mx, 2);
	  ydenom+=pow(y[i+n]-my, 2);
	  
	}
      }
      out.push_back(num);///sqrt(xdenom+ydenom));
      //      outx.push_back(time[(length/2)+n]);
      outx.push_back((double)n *timescale);
      //    n++;    
    }



  
  TGraph *outt = new TGraph(outx.size(), &outx[0], &out[0]);
  outt->GetXaxis()->SetTitle("offset (ns)");
  outt->GetYaxis()->SetTitle("CC coefficient");
  outt->GetYaxis()->SetTitleOffset(1.15);
  outt->SetTitle("");
  outt->GetXaxis()->SetRangeUser(-max_delay, max_delay);
  return outt;
}


TGraph * RUtil::align(TGraph * gr1, TGraph * gr2, double max_delay, double t_low, double t_high){
  double *x = gr1->GetY();
  double *time=gr1->GetX();
  double *y = gr2->GetY();
  int yn=gr1->GetN();
  int xn=gr2->GetN();
  int lengthx=xn;
  int lengthy=yn;
  int length=0;
  vector<double> out, outx, outy;
  double num, ynum, xdenom, ydenom, denom;
  double timescale = time[10]-time[9];

  length=lengthx<=lengthy?xn:yn;
  length=lengthx<=lengthy?xn:yn;

  double throwaway=time[xn-1]/10.;//throw away highest delays, they are unstable
  throwaway=0;
  max_delay=max_delay>time[xn-1]?time[xn-1]-throwaway:max_delay;

  t_high=t_high>=time[xn-1]?time[xn-1]:t_high;
  t_low=t_low<time[0]?time[0]:t_low;

  int max_delay_index=(1./timescale)*max_delay;
  double mx=0;
  double my=0;

  int n=0;
  double t=-max_delay;
  for(int n=-max_delay_index;n<max_delay_index;n++){
    //if(time[d]>=-max_delay&&time[d]<=max_delay){
      num=0.;
      xdenom=0.;
      ydenom=0.;
      for(int i=0;i<length;i++){
	if((i+n)>=0 && (i+n)<length && time[i]>=t_low && time[i]<=t_high){
	  
	  num+=(x[i]-mx)*(y[i+n]-my);
	  xdenom+=pow(x[i]-mx, 2);
	  ydenom+=pow(y[i+n]-my, 2);
	  
	}
      }
      out.push_back(num/sqrt(xdenom*ydenom));
      outx.push_back(time[(length/2)+n]);
      //    n++;    
    }

  double maxIndex=TMath::LocMax(out.size(), &out[0]);
  double offset=(maxIndex-(double)max_delay_index)*timescale;

  // outx.clear();
  // for(int i=0;i<xn;i++){
  //   outx.push_back(time[i]-offset);
  //   outy.push_back(y[i]);
  // }

  // TGraph *outt = new TGraph(outx.size(), &outx[0], &outy[0]);
  
  return delayGraph(gr2, -offset);
}


TGraph * RUtil::alignToOther(TGraph * gr1, TGraph * gr2, TGraph *othGr, double max_delay, double t_low, double t_high){
  double *x = gr1->GetY();
  double *time=gr1->GetX();
  double *y = gr2->GetY();
  double *yOth=othGr->GetY();
  int yn=gr1->GetN();
  int xn=gr2->GetN();
  int lengthx=xn;
  int lengthy=yn;
  int length=0;
  vector<double> out, outx, outy;
  double num, ynum, xdenom, ydenom, denom;
  double timescale = time[1]-time[0];

  length=lengthx<=lengthy?xn:yn;
  length=lengthx<=lengthy?xn:yn;

  double throwaway=time[xn-1]/10.;//throw away highest delays, they are unstable
  max_delay=max_delay>time[xn-1]?time[xn-1]-throwaway:max_delay;

  t_high=t_high>=time[xn-1]?time[xn-1]:t_high;
  t_low=t_low<time[0]?time[0]:t_low;

  int max_delay_index=(1./timescale)*max_delay;
  double mx=0;
  double my=0;

  int n=0;
  double t=-max_delay;
  for(int n=-max_delay_index;n<max_delay_index;n++){
    //if(time[d]>=-max_delay&&time[d]<=max_delay){
      num=0.;
      xdenom=0.;
      ydenom=0.;
      for(int i=0;i<length;i++){
	if((i+n)>0 && (i+n)<length && time[i]>=t_low && time[i]<=t_high){
	  
	  num+=(x[i]-mx)*(y[i+n]-my);
	  xdenom+=pow(x[i]-mx, 2);
	  ydenom+=pow(y[i+n]-my, 2);
	  
	}
      }
      out.push_back(num/sqrt(xdenom*ydenom));
      outx.push_back(time[(length/2)+n]);
      //    n++;    
    }

  double maxIndex=TMath::LocMax(out.size(), &out[0]);
  double offset=(maxIndex-(double)max_delay_index)*timescale;

  outx.clear();
  for(int i=0;i<othGr->GetN();i++){
    outx.push_back(othGr->GetX()[i]-offset);
    //    outy.push_back(y[i]);
  }

  TGraph *outt = new TGraph(outx.size(), &outx[0], yOth);
  
  return outt;
}


vector<TGraph*> RUtil::alignMultiple(vector<TGraph*> inGr, double max_delay, double t_low, double t_high){   
  vector<TGraph*>outgraphs;
  TGraph *g1=inGr[0];
  //  g1->Draw("al PLC");
  outgraphs.push_back(g1);
  for(int i=1;i<inGr.size();i++){
    outgraphs.push_back(align(g1, inGr[i], max_delay, t_low, t_high));
    //    cout<<i<<endl;
    //    outgraphs[i]->Draw("l same PLC");
  }
  return outgraphs;
}

vector<TGraph*> RUtil::alignMultiple(int N, TGraph** inGr, double max_delay, double t_low, double t_high){   
  vector<TGraph*>outgraphs;
  TGraph *g1=inGr[0];
  //  g1->Draw("al PLC");
  outgraphs.push_back(g1);
  for(int i=1;i<N;i++){
    outgraphs.push_back(align(g1, inGr[i], max_delay, t_low, t_high));
    //    cout<<i<<endl;
    //    outgraphs[i]->Draw("l same PLC");
  }
  return outgraphs;
}
TGraph* RUtil::alignMultipleAndAverage(vector<TGraph*> inGr, double max_delay, double t_low, double t_high){   
  vector<TGraph*>outgraphs;
  TGraph *g1=inGr[0];
  //  g1->Draw("al PLC");
  outgraphs.push_back(g1);
  for(int i=1;i<inGr.size();i++){
    outgraphs.push_back(align(g1, inGr[i], max_delay, t_low, t_high));
    //    cout<<i<<endl;
    //    outgraphs[i]->Draw("l same PLC");
  }
  auto avgGr=RUtil::avgGraph(outgraphs);
  outgraphs.clear();
  
  return avgGr;
}


vector<TGraph*> RUtil::alignMultipleAndTruncate(vector<TGraph*> inGr, double max_delay, double t_min, double t_max, double t_low, double t_high){   
  vector<TGraph*>tempgraphs;
  vector<TGraph*>outgraphs;
  TGraph *g1=inGr[0];
  //  g1->Draw("al PLC");
  tempgraphs.push_back(g1);
  for(int i=1;i<inGr.size();i++){
    auto gr=align(g1, inGr[i], max_delay, t_low, t_high);
    tempgraphs.push_back(gr);
    //    cout<<i<<endl;
    //    outgraphs[i]->Draw("l same PLC");
  }
  for(int i=0;i<tempgraphs.size();i++){
    outgraphs.push_back(RUtil::getChunkOfGraph(tempgraphs[i], t_min, t_max, 1));
  }
  tempgraphs.clear();
  return outgraphs;
}

vector<TGraph*> RUtil::alignMultipleToOther(vector<TGraph*> inGr, vector<TGraph*> othGr, double max_delay, double t_low, double t_high){   
  vector<TGraph*>outgraphs;
  TGraph *g1=inGr[0];
  //  g1->Draw("al PLC");
  outgraphs.push_back(othGr[0]);
  for(int i=1;i<inGr.size();i++){
    outgraphs.push_back(alignToOther(g1, inGr[i], othGr[i], max_delay, t_low, t_high));
    //    cout<<i<<endl;
    //    outgraphs[i]->Draw("l same PLC");
  }
  return outgraphs;
}

TGraph * RUtil::zeros(int len, double dt){
  auto og=new TGraph(len);
  for(int i=0;i<len;i++){
    og->SetPoint(i, (double)i*dt, 0.);
  }
  return og;
}

TGraph * RUtil::zeros(double start, double stop, int len){
  double dx=(stop-start)/len;
  auto og=new TGraph(len);
  for(int i=0;i<len;i++){
    og->SetPoint(i, start+(double)i*dx, 0.);
  }
  return og;
}

TGraph * RUtil::ones(int len, double dt){
  auto og=new TGraph(len);
  for(int i=0;i<len;i++){
    og->SetPoint(i, (double)i*dt, 1.);
  }
  return og;
}

TGraph * RUtil::ones(double start, double stop, int len){
  double dx=(stop-start)/len;
  auto og=new TGraph(len);
  for(int i=0;i<len;i++){
    og->SetPoint(i, start+(double)i*dx, 1.);
  }
  return og;
}

int RUtil::fillCWArray(double * arr, int N, double freq, double amp, double dt, double phase){
  double t=0.;
  for(int i=0;i<N;i++){
    arr[i]=amp*sin(2.*pi*freq*t +phase);
    t+=dt;
  }
  return N; 
}

TGraph * RUtil::makeCW(double freq,  double amp, double t_min, double t_max, double GSs, double phase){
  
  int n=(t_max-t_min)*GSs;
  TGraph * oG=new TGraph();
  double dt=1./GSs;
  double t=t_min-(10.*dt);
  while(t<=t_max+(10.*dt)){
    double temp=amp*sin(2.*pi*freq*t + phase);
    oG->SetPoint(oG->GetN(), t, temp);
    t+=dt;
  }
  auto slice=RUtil::getChunkOfGraph(oG, t_min, t_max);
  delete oG;
  return slice;
}

TGraph * RUtil::sampledCW(double freq,  double amp, int N, double * times, double phase){

  TGraph * oG=new TGraph();
  for(int i=0;i<N;i++){
    double temp=amp*sin(2.*pi*freq*times[i] + phase);
    oG->SetPoint(oG->GetN(), times[i], temp);
  }
  return oG;
}

TGraph * RUtil::sampledCW(double freq,  double amp, vector<double> times, double phase){
  TGraph * oG=new TGraph();
  for(int i=0;i<times.size();i++){
    double temp=amp*sin(2.*pi*freq*times[i] + phase);
    oG->SetPoint(oG->GetN(), times[i], temp);
  }
  return oG;
}


TGraph * RUtil::lowpassFilter(TGraph *ingr, double cutoff, int order){
  double * yy=ingr->GetY();
  double *xx= ingr->GetX();
  int n = ingr->GetN();
  vector<double> outx, outy;
  
  double w = cutoff*2.*pi*1.e9;
  double T = (xx[10]-xx[9])*1.e-9;;
  double a, b, c, value;
  
  //cout<<setprecision(12);
  //cout<<"filter coefficients"<<endl<<a<<endl<<b<<endl<<c<<endl;
  //  int size = in.size();
  if(order==1){
    //    a = w*T;
    b = exp(-w*T);
    a=1.-b;
    
    for(int i=0;i<n;i++){
      if(i>0){
	
	value = a*yy[i]+b*outy[i-1];
	
	outy.push_back(value);
      }
      if(i==0){
	value = a*yy[i];
	//value=0.;
	outy.push_back(value);
      } 
    }
  }
  
  else{
    //a = pow(T, 2.)*pow(w, 2.)*exp(-2.*w*T);
    b = 2.*exp(-w*T);
    c = exp(-2.*w*T);
    a=1.-(b-c);	
    //	cout<<"filter coefficients"<<endl<<a<<endl<<b<<endl<<c<<endl;

    for(int i=0;i<n;i++){
      if(i>1){
	value = a*yy[i-1]+b*outy[i-1]-c*outy[i-2];
	outy.push_back(value);

      }
      if(i==1){
	value = a*yy[i-1]+b*outy[i-1];
	outy.push_back(value);
			
      }
      if(i==0){
	outy.push_back(0.);
      } 
      // outx.push_back(xx[i]);
    }
  }
  TGraph *outgr = new TGraph(n, ingr->GetX(), &outy[0]);
  return outgr;
}


TGraph * RUtil::highpassFilter(TGraph *ingr, double cutoff, int order){
  double * yy=ingr->GetY();
  double *xx= ingr->GetX();
  int n = ingr->GetN();
  vector<double> outx, outy;
  
  double w = cutoff*2.*pi*1.e9;
  double T = (xx[10]-xx[9])*1.e-9;;
  double a, b, c, value, x;
  x=exp(-w*T);  
  //cout<<setprecision(12);
  //cout<<"filter coefficients"<<endl<<a<<endl<<b<<endl<<c<<endl;
  //  int size = in.size();
  if(order==1){
    //a = pow(T, 2.)*pow(w, 2.)*exp(-2.*w*T);
    auto a0 = (1.+x)/2.;
    auto a1 = -(1.+x)/2.;
    auto b1 = x	;
    //	cout<<"filter coefficients"<<endl<<a<<endl<<b<<endl<<c<<endl;

    for(int i=0;i<n;i++){
      if(i>1){
	value = a0*yy[i-1]+a1*yy[i-2]+b1*outy[i-1];
	outy.push_back(value);

      }
      if(i==1){
	value = a0*yy[i-1]+b1*outy[i-1];
	outy.push_back(value);
			
      }
      if(i==0){
	outy.push_back(0.);
      } 
      // outx.push_back(xx[i]);
    }
  }
  TGraph *outgr = new TGraph(n, ingr->GetX(), &outy[0]);
  return outgr;
}

TGraph * RUtil::bandpassFilter(TGraph *inGr, double low, double high){
  auto lp=RUtil::lowpassFilter(inGr, high);
  auto hp=RUtil::highpassFilter(lp, low);
  delete lp;
  return hp;
}

TGraph * RUtil::brickWallFilter(TGraph * inGr, double low, double high){
  
  auto fT=RUtil::FFT::fft(inGr);

  double fs=1./(inGr->GetX()[1]-inGr->GetX()[0]);
  double df=fs/(double)inGr->GetN();

  int indL=low/df;
  int indH=high/df;
  //cout<<fs<<" "<<indL<<" "<<indH<<endl;
  for(int i=0;i<indL;i++){
    fT->SetPoint(i, fT->GetX()[i], 0., 0.);
  }

  for(int i=indH;i<inGr->GetN();i++){
    fT->SetPoint(i, fT->GetX()[i], 0., 0.);
  }

  auto outGr=(TGraph*)RUtil::FFT::ifft(fT)->Clone();
  return outGr;
}

TGraph * RUtil::removeCW(TGraph *ingr, double freq){
  double * y=ingr->GetY();
  double * x=ingr->GetX();
  double dt=x[10]-x[9];
  int N=ingr->GetN();
  double sine[N];
  double cosine[N];
  double sine_normalized[N];
  double cosine_normalized[N];
  double y_normalized[N];
  RUtil::fillCWArray(sine, N,freq, 1, dt,0);
  RUtil::fillCWArray(cosine, N,freq, 1,dt, RUtil::deg2Rad(90));
  auto norm_of_y=RUtil::normalize(y_normalized,N,  y);
  RUtil::normalize(sine_normalized, N, sine);
  RUtil::normalize(cosine_normalized, N, cosine);
  auto coef0=RUtil::dot(y_normalized, sine_normalized, N);
  auto coef1=RUtil::dot(y_normalized, cosine_normalized, N);
  double y_temp0[N];
  double y_temp1[N];
  double result[N];
  double sine_scaled[N];
  double cosine_scaled[N];
  RUtil::scale(sine_scaled, N, sine_normalized, coef0);
  RUtil::scale(cosine_scaled, N, cosine_normalized, coef1);
  RUtil::add(y_temp0,N, y_normalized, sine_scaled, -1);
  RUtil::add(y_temp1,N, y_temp0, cosine_scaled, -1);
  RUtil::scale(result,N, y_temp1, norm_of_y);
  auto ind=RUtil::makeIndices(N,dt);
  auto gr=new TGraph(N, ind, result);
  delete ind;
  return gr;
  
}


TGraph * RUtil::addNoise(TGraph * inGr, double level, TString type){
  //  cout<<type<<endl;
  auto outGr=new TGraph();
  auto rand=new TRandom3();
  rand->SetSeed();
  for(int i=0;i<inGr->GetN();i++){
    auto ranN=0.;
    if(type.Contains("Poisson")){
      ranN=rand->PoissonD(level);
    }
    else if(type.Contains("Gaus")){
      ranN=rand->Gaus(0, level);
    }
    else if(type.Contains("Uniform")){
      ranN=rand->Uniform(-level, level);
    }
    //else{
    //ranN=rand->Uniform(-level, level);
    //}
    outGr->SetPoint(outGr->GetN(), inGr->GetX()[i], inGr->GetY()[i]+ranN);
  }
  delete rand;
  return outGr;
}

TGraph * RUtil::makeNullData(TGraph *sig, TGraph *back, double t_min, double t_max, double scale){
  auto sigchunk=getChunkOfGraph(sig, 0., (t_max-t_min));
  auto backchunk=getChunkOfGraph(back, t_min, t_max ,1);
  sigchunk->SetBit(kCanDelete);
  backchunk->SetBit(kCanDelete);
  //auto outg=add(sigchunk, RUtil::scale(backchunk, scale));
  auto outg=add(sigchunk, backchunk);
  delete sigchunk;
  delete backchunk;
  return outg;
}

TGraph * RUtil::makeNullDataFixedLength(TGraph *sig, TGraph *back, double t_min, int nSamps){
  auto sigchunk=getNSamplesFrom(sig, 0., nSamps, 0);
  auto backchunk=getNSamplesFrom(back, t_min, nSamps ,1);
  return add(sigchunk, backchunk);
}


double RUtil::integrate(TH2D *h, double xmin, double xmax, double ymin, double ymax){
  double err=0.;
  Int_t xmin_bin = h->GetXaxis()->FindBin(xmin);
  Int_t xmax_bin = h->GetXaxis()->FindBin(xmax);
  Int_t ymin_bin = h->GetYaxis()->FindBin(ymin);
  Int_t ymax_bin = h->GetYaxis()->FindBin(ymax);

  return  h->IntegralAndError(xmin_bin, xmax_bin, ymin_bin, ymax_bin, err);
  
}

double RUtil::integrateWithError(TH2D *h, double xmin, double xmax, double ymin, double ymax, double & err){
  Int_t xmin_bin = h->GetXaxis()->FindBin(xmin);
  Int_t xmax_bin = h->GetXaxis()->FindBin(xmax);
  Int_t ymin_bin = h->GetYaxis()->FindBin(ymin);
  Int_t ymax_bin = h->GetYaxis()->FindBin(ymax);
  //  cout<<xmin_bin<<" "<<xmax_bin<<" "<<ymin_bin<<" "<<ymax_bin<<endl;
  return  h->IntegralAndError(xmin_bin, xmax_bin, ymin_bin, ymax_bin, err);//RUtil::integrate(h, xmin, xmax, ymin, ymax, err);
}


// double integral_1d(TH1F *h, double xmin, double xmax, double & err){
//   Int_t xmin_bin = h->GetXaxis()->FindBin(xmin);
//   Int_t xmax_bin = h->GetXaxis()->FindBin(xmax);
 

//   return  h->IntegralAndError(xmin_bin, xmax_bin, err);
  
// }

/* 
here is the layout for the coordinates used. centermost band is the 
signal band. those to either side in both dims are the sidebands. the integrals
ix1, iy2 etc are the integrals of those quadrants. ib1-4 are averaged
for the overall background. ix1-background and ix2-background are averaged
to get the signal band background, same in y. finally, background and both 
signal band backgrounds are subtracted from the signal quadrant to get signal. 
   __________________________
  |      |   |   |   |       |
  |______|___|___|___|_______|y4 
  |      |ib3|iy2|ib4|       | 
  |______|___|___|___|_______|y3
  |      |ix1|sig|ix2|       | 
  |______|___|___|___|_______|y2
  |      |ib1|iy1|ib2|       | 
  |______|___|___|___|_______|y1
  |      |   |   |   |       | 
  |      |   |   |   |       | 
  |______|___|___|___|_______| 
        x1   x2  x3  x4 
    


 */


double RUtil::sidebandSubtraction2DWithErrors(TH2D *h, double sband_x1, double sband_x2, double sband_y1, double sband_y2, double & err, int draw, Color_t color, double alpha){

  double x1, x2, x3, x4, y1, y2, y3, y4, bandwidth_x, bandwidth_y,ix1, ix2, iy1, iy2, isig, ib1, ib2, ib3, ib4, background, bandy, bandx, avg_x, avg_y, sig;
  double ix1_err, ix2_err, iy1_err, iy2_err, isig_err, ib1_err, ib2_err, ib3_err, ib4_err, background_err, bandy_err, bandx_err, avg_x_err, avg_y_err, sig_err;
  //assign the cordinates of the signal and sidebands
  x2 = sband_x1;
  x3 = sband_x2;
  y2 = sband_y1;
  y3 = sband_y2;
  bandwidth_x = x3-x2;
  bandwidth_y = y3-y2;
  x1 = x2-bandwidth_x;
  x4 = x3+bandwidth_x;
  y1 = y2-bandwidth_y;
  y4 = y3+bandwidth_y;
  //make the background integrals and average
  ib1 = integrateWithError(h, x1, x2, y1, y2, ib1_err);
  ib2 = integrateWithError(h, x3, x4, y1, y2, ib2_err);
  ib3 = integrateWithError(h, x1, x2, y3, y4, ib3_err);
  ib4 = integrateWithError(h, x3, x4, y3, y4, ib4_err);

  background =(ib1+ib2+ib3+ib4)/4;

  //make the signal band backgrounds
  ix1 = integrateWithError(h, x1, x2, y2, y3, ix1_err);
  ix2 = integrateWithError(h, x3, x4, y2, y3, ix2_err);
  iy1 = integrateWithError(h, x2, x3, y1, y2, iy1_err);
  iy2 = integrateWithError(h, x2, x3, y3, y4, iy2_err);

   bandx = ((ix1-background)+(ix2-background))/2.;
   bandy = ((iy1-background)+(iy2-background))/2.;
 
 
  //make the signal integral
  isig = integrateWithError(h, x2, x3, y2, y3, isig_err);
  
  //cout<<isig<<" "<<isig_err<<endl;
  //cout<<ix1<<" "<<ix1_err<<endl;
  //cout<<iy1<<" "<<iy1_err<<endl;
  //errors. add in quadrature and average.
  background_err = sqrt(pow(ib1_err, 2)+pow(ib2_err, 2)+pow(ib3_err, 2)+pow(ib4_err, 2))/4.;
  bandx_err = sqrt(pow(ix1_err, 2)+pow(ix2_err, 2))/2.;
  bandy_err = sqrt(pow(iy1_err, 2)+pow(iy2_err, 2))/2.;


  // background_err = (ib1_err+ib2_err+ib3_err+ib4_err)/4.;
  // bandx_err = (ix1_err+ix2_err)/2.;
  // bandy_err = (iy1_err+iy2_err)/2.;

  sig_err = sqrt(pow(background_err, 2)+pow(bandx_err, 2)+pow(bandy_err, 2));
  err=sig_err;
  //construct signal
  sig = isig-(background+bandx+bandy);
  //  sig = isig-((ix1+ix2+iy1+iy2)/4);

  //  cout<<"total events: "<<sig<<"+/-"<<sig_err<<endl;
  //  TString tit = std::to_string(sig);

  if(draw==1){
    h->Draw("colz0");
    double ymin = gPad->GetUymin();//h->GetYaxis()->GetXmin();
    double ymax = gPad->GetUymax();//h->GetYaxis()->GetXmax();
    double xmin = gPad->GetUxmin();//h->GetXaxis()->GetXmin();
    double xmax = gPad->GetUxmax();//h->GetXaxis()->GetXmax();

    TLine *l1 = new TLine(x2, ymin, x2, ymax);
    TLine *l2 = new TLine(x3, ymin, x3, ymax);
    TLine *l3 = new TLine(xmin, y2, xmax, y2);
    TLine *l4 = new TLine(xmin, y3, xmax, y3);
    TLine *l5 = new TLine(x1, ymin, x1, ymax);
    TLine *l6 = new TLine(x4, ymin, x4, ymax);
    TLine *l7 = new TLine(xmin, y1, xmax, y1);
    TLine *l8 = new TLine(xmin, y4, xmax, y4);
    l1->SetLineColorAlpha(color, alpha);
    l2->SetLineColorAlpha(color, alpha);
    l3->SetLineColorAlpha(color, alpha);
    l4->SetLineColorAlpha(color, alpha);

    l5->SetLineColorAlpha(color, alpha);
    l6->SetLineColorAlpha(color, alpha);
    l7->SetLineColorAlpha(color, alpha);
    l8->SetLineColorAlpha(color, alpha);
    l5->SetLineStyle(7);
    l6->SetLineStyle(7);
    l7->SetLineStyle(7);
    l8->SetLineStyle(7);
    
    l1->Draw();
    l2->Draw();
    l3->Draw();
    l4->Draw();
    l5->Draw();
    l6->Draw();
    l7->Draw();
    l8->Draw();
    char title[100];

    h->SetStats(0);
    ((TCanvas*)gROOT->GetListOfCanvases()->At(0))->SetRightMargin(0.15);
  }


  
  return sig;
}


double RUtil::sidebandSubtractionXAxisWithErrors(TH2D *h, double sband_x1, double sband_x2, double sband_y1, double sband_y2, double & err, int draw, Color_t color){

  double x1, x2, x3, x4, y1, y2, y3, y4, bandwidth_x, bandwidth_y,ix1, ix2, iy1, iy2, isig, ib1, ib2, ib3, ib4, background, bandy, bandx, avg_x, avg_y, sig;
  double ix1_err, ix2_err, iy1_err, iy2_err, isig_err, ib1_err, ib2_err, ib3_err, ib4_err, background_err, bandy_err, bandx_err, avg_x_err, avg_y_err, sig_err;
  //assign the cordinates of the signal and sidebands
  x2 = sband_x1;
  x3 = sband_x2;
  y2 = sband_y1;
  y3 = sband_y2;
  bandwidth_x = x3-x2;
  bandwidth_y = y3-y2;
  x1 = x2-bandwidth_x;
  x4 = x3+bandwidth_x;
  y1 = y2-bandwidth_y;
  y4 = y3+bandwidth_y;
  //make the background integrals and average
  // ib1 = integrateWithError(h, x1, x2, y1, y2, ib1_err);
  // ib2 = integrateWithError(h, x3, x4, y1, y2, ib2_err);
  // ib3 = integrateWithError(h, x1, x2, y3, y4, ib3_err);
  // ib4 = integrateWithError(h, x3, x4, y3, y4, ib4_err);

  // background =(ib1+ib2+ib3+ib4)/4;

  //make the signal band backgrounds
  ix1 = integrateWithError(h, x1, x2, y2, y3, ix1_err);
  ix2 = integrateWithError(h, x3, x4, y2, y3, ix2_err);
  //  iy1 = integrateWithError(h, x2, x3, y1, y2, iy1_err);
  //iy2 = integrateWithError(h, x2, x3, y3, y4, iy2_err);

   bandx = ((ix1)+(ix2))/2.;
   // bandy = ((iy1-background)+(iy2-background))/2.;
 
 
  //make the signal integral
  isig = integrateWithError(h, x2, x3, y2, y3, isig_err);
  
  //cout<<isig<<" "<<isig_err<<endl;
  //cout<<ix1<<" "<<ix1_err<<endl;
  //cout<<iy1<<" "<<iy1_err<<endl;
  //errors. add in quadrature and average.
  //background_err = sqrt(pow(ib1_err, 2)+pow(ib2_err, 2)+pow(ib3_err, 2)+pow(ib4_err, 2))/4.;
  bandx_err = sqrt(pow(ix1_err, 2)+pow(ix2_err, 2))/2.;
  //bandy_err = sqrt(pow(iy1_err, 2)+pow(iy2_err, 2))/2.;


  // background_err = (ib1_err+ib2_err+ib3_err+ib4_err)/4.;
  // bandx_err = (ix1_err+ix2_err)/2.;
  // bandy_err = (iy1_err+iy2_err)/2.;

  sig_err = sqrt(pow(bandx_err, 2));
  err=sig_err;
  //construct signal
  sig = isig-(bandx);
  //  sig = isig-((ix1+ix2+iy1+iy2)/4);

  //  cout<<"total events: "<<sig<<"+/-"<<sig_err<<endl;
  //  TString tit = std::to_string(sig);

  if(draw==1){
    h->Draw("colz0");
    double ymin = gPad->GetUymin();//h->GetYaxis()->GetXmin();
    double ymax = gPad->GetUymax();//h->GetYaxis()->GetXmax();
    double xmin = gPad->GetUxmin();//h->GetXaxis()->GetXmin();
    double xmax = gPad->GetUxmax();//h->GetXaxis()->GetXmax();

    TLine *l1 = new TLine(x2, ymin, x2, ymax);
    TLine *l2 = new TLine(x3, ymin, x3, ymax);
    TLine *l3 = new TLine(xmin, y2, xmax, y2);
    TLine *l4 = new TLine(xmin, y3, xmax, y3);
    TLine *l5 = new TLine(x1, ymin, x1, ymax);
    TLine *l6 = new TLine(x4, ymin, x4, ymax);
    TLine *l7 = new TLine(xmin, y1, xmax, y1);
    TLine *l8 = new TLine(xmin, y4, xmax, y4);
    l1->SetLineColor(color);
    l2->SetLineColor(color);
    l3->SetLineColor(color);
    l4->SetLineColor(color);

    l5->SetLineColor(color);
    l6->SetLineColor(color);
    l7->SetLineColor(color);
    l8->SetLineColor(color);
    l5->SetLineStyle(7);
    l6->SetLineStyle(7);
    l7->SetLineStyle(7);
    l8->SetLineStyle(7);
    
    l1->Draw();
    l2->Draw();
    l3->Draw();
    l4->Draw();
    l5->Draw();
    l6->Draw();
    l7->Draw();
    l8->Draw();
    char title[100];

    h->SetStats(0);
    ((TCanvas*)gROOT->GetListOfCanvases()->At(0))->SetRightMargin(0.15);
  }


  
  return sig;
}

double RUtil::sidebandSubtractionYAxisWithErrors(TH2D *h, double sband_x1, double sband_x2, double sband_y1, double sband_y2, double & err, int draw, Color_t color){

  double x1, x2, x3, x4, y1, y2, y3, y4, bandwidth_x, bandwidth_y,ix1, ix2, iy1, iy2, isig, ib1, ib2, ib3, ib4, background, bandy, bandx, avg_x, avg_y, sig;
  double ix1_err, ix2_err, iy1_err, iy2_err, isig_err, ib1_err, ib2_err, ib3_err, ib4_err, background_err, bandy_err, bandx_err, avg_x_err, avg_y_err, sig_err;
  //assign the cordinates of the signal and sidebands
  x2 = sband_x1;
  x3 = sband_x2;
  y2 = sband_y1;
  y3 = sband_y2;
  bandwidth_x = x3-x2;
  bandwidth_y = y3-y2;
  x1 = x2-bandwidth_x;
  x4 = x3+bandwidth_x;
  y1 = y2-bandwidth_y;
  y4 = y3+bandwidth_y;
  //make the background integrals and average
  // ib1 = integrateWithError(h, x1, x2, y1, y2, ib1_err);
  // ib2 = integrateWithError(h, x3, x4, y1, y2, ib2_err);
  // ib3 = integrateWithError(h, x1, x2, y3, y4, ib3_err);
  // ib4 = integrateWithError(h, x3, x4, y3, y4, ib4_err);

  // background =(ib1+ib2+ib3+ib4)/4;

  //make the signal band backgrounds
  //ix1 = integrateWithError(h, x1, x2, y2, y3, ix1_err);
  //ix2 = integrateWithError(h, x3, x4, y2, y3, ix2_err);
  iy1 = integrateWithError(h, x2, x3, y1, y2, iy1_err);
  iy2 = integrateWithError(h, x2, x3, y3, y4, iy2_err);

  //bandx = ((ix1)+(ix2))/2.;
  bandy = ((iy1)+(iy2))/2.;
 
 
  //make the signal integral
  isig = integrateWithError(h, x2, x3, y2, y3, isig_err);
  
  //cout<<isig<<" "<<isig_err<<endl;
  //cout<<ix1<<" "<<ix1_err<<endl;
  //cout<<iy1<<" "<<iy1_err<<endl;
  //errors. add in quadrature and average.
  //background_err = sqrt(pow(ib1_err, 2)+pow(ib2_err, 2)+pow(ib3_err, 2)+pow(ib4_err, 2))/4.;
  //bandx_err = sqrt(pow(ix1_err, 2)+pow(ix2_err, 2))/2.;
  bandy_err = sqrt(pow(iy1_err, 2)+pow(iy2_err, 2))/2.;


  // background_err = (ib1_err+ib2_err+ib3_err+ib4_err)/4.;
  // bandx_err = (ix1_err+ix2_err)/2.;
  // bandy_err = (iy1_err+iy2_err)/2.;

  sig_err = sqrt(pow(bandy_err, 2));
  err=sig_err;
  //construct signal
  sig = isig-(bandy);
  //  sig = isig-((ix1+ix2+iy1+iy2)/4);

  //  cout<<"total events: "<<sig<<"+/-"<<sig_err<<endl;
  //  TString tit = std::to_string(sig);

  if(draw==1){
    h->Draw("colz0");
    double ymin = gPad->GetUymin();//h->GetYaxis()->GetXmin();
    double ymax = gPad->GetUymax();//h->GetYaxis()->GetXmax();
    double xmin = gPad->GetUxmin();//h->GetXaxis()->GetXmin();
    double xmax = gPad->GetUxmax();//h->GetXaxis()->GetXmax();

    TLine *l1 = new TLine(x2, ymin, x2, ymax);
    TLine *l2 = new TLine(x3, ymin, x3, ymax);
    TLine *l3 = new TLine(xmin, y2, xmax, y2);
    TLine *l4 = new TLine(xmin, y3, xmax, y3);
    TLine *l5 = new TLine(x1, ymin, x1, ymax);
    TLine *l6 = new TLine(x4, ymin, x4, ymax);
    TLine *l7 = new TLine(xmin, y1, xmax, y1);
    TLine *l8 = new TLine(xmin, y4, xmax, y4);
    l1->SetLineColor(color);
    l2->SetLineColor(color);
    l3->SetLineColor(color);
    l4->SetLineColor(color);

    l5->SetLineColor(color);
    l6->SetLineColor(color);
    l7->SetLineColor(color);
    l8->SetLineColor(color);
    l5->SetLineStyle(7);
    l6->SetLineStyle(7);
    l7->SetLineStyle(7);
    l8->SetLineStyle(7);
    
    l1->Draw();
    l2->Draw();
    l3->Draw();
    l4->Draw();
    l5->Draw();
    l6->Draw();
    l7->Draw();
    l8->Draw();
    char title[100];

    h->SetStats(0);
    ((TCanvas*)gROOT->GetListOfCanvases()->At(0))->SetRightMargin(0.15);
  }


  
  return sig;
} 


double RUtil::sidebandSubtraction2D(TH2D *h, double sband_x1, double sband_x2, double sband_y1, double sband_y2, int draw, Color_t color){

  double x1, x2, x3, x4, y1, y2, y3, y4, bandwidth_x, bandwidth_y,ix1, ix2, iy1, iy2, isig, ib1, ib2, ib3, ib4, background, bandy, bandx, avg_x, avg_y, sig;
  double ix1_err, ix2_err, iy1_err, iy2_err, isig_err, ib1_err, ib2_err, ib3_err, ib4_err, background_err, bandy_err, bandx_err, avg_x_err, avg_y_err, sig_err;
  //assign the cordinates of the signal and sidebands
  x2 = sband_x1;
  x3 = sband_x2;
  y2 = sband_y1;
  y3 = sband_y2;
  bandwidth_x = x3-x2;
  bandwidth_y = y3-y2;
  x1 = x2-bandwidth_x;
  x4 = x3+bandwidth_x;
  y1 = y2-bandwidth_y;
  y4 = y3+bandwidth_y;
  //make the background integrals and average
  ib1 = integrateWithError(h, x1, x2, y1, y2, ib1_err);
  ib2 = integrateWithError(h, x3, x4, y1, y2, ib2_err);
  ib3 = integrateWithError(h, x1, x2, y3, y4, ib3_err);
  ib4 = integrateWithError(h, x3, x4, y3, y4, ib4_err);

  background =(ib1+ib2+ib3+ib4)/4;

  //make the signal band backgrounds
  ix1 = integrateWithError(h, x1, x2, y2, y3, ix1_err);
  ix2 = integrateWithError(h, x3, x4, y2, y3, ix2_err);
  iy1 = integrateWithError(h, x2, x3, y1, y2, iy1_err);
  iy2 = integrateWithError(h, x2, x3, y3, y4, iy2_err);

   bandx = ((ix1-background)+(ix2-background))/2.;
   bandy = ((iy1-background)+(iy2-background))/2.;
 
 
  //make the signal integral
  isig = integrateWithError(h, x2, x3, y2, y3, isig_err);
  

  //errors. add in quadrature and average.
  background_err = sqrt(pow(ib1_err, 2)+pow(ib2_err, 2)+pow(ib3_err, 2)+pow(ib4_err, 2))/2;
  bandx_err = sqrt(pow(ix1_err, 2)+pow(ix2_err, 2))/sqrt(2);
  bandy_err = sqrt(pow(iy1_err, 2)+pow(iy2_err, 2))/sqrt(2);

  sig_err = sqrt(pow(background_err, 2)+pow(bandx_err, 2)+pow(bandy_err, 2));
  //  err=sig_err;
  //construct signal
  sig = isig-(background+bandx+bandy);
  //  sig = isig-((ix1+ix2+iy1+iy2)/4);

  //  cout<<"total events: "<<sig<<"+/-"<<sig_err<<endl;
  //  TString tit = std::to_string(sig);

  if(draw==1){
    h->Draw("colz0");
    double ymin = gPad->GetUymin();//h->GetYaxis()->GetXmin();
    double ymax = gPad->GetUymax();//h->GetYaxis()->GetXmax();
    double xmin = gPad->GetUxmin();//h->GetXaxis()->GetXmin();
    double xmax = gPad->GetUxmax();//h->GetXaxis()->GetXmax();

    TLine *l1 = new TLine(x2, ymin, x2, ymax);
    TLine *l2 = new TLine(x3, ymin, x3, ymax);
    TLine *l3 = new TLine(xmin, y2, xmax, y2);
    TLine *l4 = new TLine(xmin, y3, xmax, y3);
    TLine *l5 = new TLine(x1, ymin, x1, ymax);
    TLine *l6 = new TLine(x4, ymin, x4, ymax);
    TLine *l7 = new TLine(xmin, y1, xmax, y1);
    TLine *l8 = new TLine(xmin, y4, xmax, y4);
    l1->SetLineColor(color);
    l2->SetLineColor(color);
    l3->SetLineColor(color);
    l4->SetLineColor(color);

    l5->SetLineColor(color);
    l6->SetLineColor(color);
    l7->SetLineColor(color);
    l8->SetLineColor(color);
    l5->SetLineStyle(7);
    l6->SetLineStyle(7);
    l7->SetLineStyle(7);
    l8->SetLineStyle(7);

    l1->Draw();
    l2->Draw();
    l3->Draw();
    l4->Draw();
    l5->Draw();
    l6->Draw();
    l7->Draw();
    l8->Draw();
    char title[100];

    h->SetStats(0);
    ((TCanvas*)gROOT->GetListOfCanvases()->At(0))->SetRightMargin(0.15);
  }


  
  return sig;
} 

double RUtil::sidebandSubtraction2DDev(TH2D *h, double sband_x1, double sband_x2, double sband_y1, double sband_y2, double & err, int draw, Color_t color){

  double x1, x2, x3, x4, y1, y2, y3, y4, bandwidth_x, bandwidth_y,ix1, ix2, iy1, iy2, isig, ib1, ib2, ib3, ib4, background, bandy, bandx, avg_x, avg_y, sig;
  double ix1_err, ix2_err, iy1_err, iy2_err, isig_err, ib1_err, ib2_err, ib3_err, ib4_err, background_err, bandy_err, bandx_err, avg_x_err, avg_y_err, sig_err;
  //assign the cordinates of the signal and sidebands
  x2 = sband_x1;
  x3 = sband_x2;
  y2 = sband_y1;
  y3 = sband_y2;
  bandwidth_x = x3-x2;
  bandwidth_y = y3-y2;
  x1 = x2-bandwidth_x;
  x4 = x3+bandwidth_x;
  y1 = y2-bandwidth_y;
  y4 = y3+bandwidth_y;

  //make the signal band backgrounds
  ix1 = integrateWithError(h, x1, x2, y2, y3, ix1_err);
  ix2 = integrateWithError(h, x3, x4, y2, y3, ix2_err);
  iy1 = integrateWithError(h, x2, x3, y1, y2, iy1_err);
  iy2 = integrateWithError(h, x2, x3, y3, y4, iy2_err);

  bandx = ((ix1)+(ix2))/2.;
  bandy = ((iy1)+(iy2))/2.;
 
 
  //make the signal integral
  isig = integrateWithError(h, x2, x3, y2, y3, isig_err);
  
  //cout<<isig<<" "<<isig_err<<endl;
  //cout<<ix1<<" "<<ix1_err<<endl;
  //cout<<iy1<<" "<<iy1_err<<endl;
  //errors. add in quadrature and average.
  bandx_err = sqrt(pow(ix1_err, 2)+pow(ix2_err, 2))/2.;
  bandy_err = sqrt(pow(iy1_err, 2)+pow(iy2_err, 2))/2.;


  // background_err = (ib1_err+ib2_err+ib3_err+ib4_err)/4.;
  // bandx_err = (ix1_err+ix2_err)/2.;
  // bandy_err = (iy1_err+iy2_err)/2.;

  sig_err = sqrt(pow(bandx_err, 2)+pow(bandy_err, 2));
  err=sig_err;
  //construct signal
  sig = isig-((bandx+bandy)/2.);
  //  sig = isig-((ix1+ix2+iy1+iy2)/4);

  //  cout<<"total events: "<<sig<<"+/-"<<sig_err<<endl;
  //  TString tit = std::to_string(sig);

  if(draw==1){
    h->Draw("colz0");
    double ymin = gPad->GetUymin();//h->GetYaxis()->GetXmin();
    double ymax = gPad->GetUymax();//h->GetYaxis()->GetXmax();
    double xmin = gPad->GetUxmin();//h->GetXaxis()->GetXmin();
    double xmax = gPad->GetUxmax();//h->GetXaxis()->GetXmax();

    TLine *l1 = new TLine(x2, ymin, x2, ymax);
    TLine *l2 = new TLine(x3, ymin, x3, ymax);
    TLine *l3 = new TLine(xmin, y2, xmax, y2);
    TLine *l4 = new TLine(xmin, y3, xmax, y3);
    TLine *l5 = new TLine(x1, ymin, x1, ymax);
    TLine *l6 = new TLine(x4, ymin, x4, ymax);
    TLine *l7 = new TLine(xmin, y1, xmax, y1);
    TLine *l8 = new TLine(xmin, y4, xmax, y4);
    l1->SetLineColor(color);
    l2->SetLineColor(color);
    l3->SetLineColor(color);
    l4->SetLineColor(color);

    l5->SetLineColor(color);
    l6->SetLineColor(color);
    l7->SetLineColor(color);
    l8->SetLineColor(color);
    l5->SetLineStyle(7);
    l6->SetLineStyle(7);
    l7->SetLineStyle(7);
    l8->SetLineStyle(7);
    
    l1->Draw();
    l2->Draw();
    l3->Draw();
    l4->Draw();
    l5->Draw();
    l6->Draw();
    l7->Draw();
    l8->Draw();
    char title[100];

    h->SetStats(0);
    ((TCanvas*)gROOT->GetListOfCanvases()->At(0))->SetRightMargin(0.15);
  }


  
  return sig;
} 


vector<double> RUtil::linspace(double start, double stop, int N){
  auto dt=(stop-start)/(double)N;
  auto vec=vector<double>(N);
  for(int i=0;i<N;i++){
    vec[i]=start+(double)i*dt;
  }
  return vec;
}


//these are all from wikipedia.
double RUtil::window(int i, int n, int type){
  switch (type){
    case 0:
      return bartlettWindow(i, n);
    case 1:
      return welchWindow(i, n);
    case 2:
      return hannWindow(i, n);
    case 3:
      return blackmanNuttallWindow(i, n);
    default:
      return 1.;
      break;
    }
}

double RUtil::bartlettWindow(int i, int n){
  return 1.-abs((2.*(double)i-(double)n)/((double)n));
}

double RUtil::welchWindow(int i, int n){
  return 1.-pow((2.*(double)i-(double)n)/((double)n), 2);
}

double RUtil::hannWindow(int i, int n){
  return sin(pi*i/(n))*sin(pi*i/(n));
}

double RUtil::blackmanNuttallWindow(int i, int n){
  return .3635819-.4891775*cos(2.*pi*i/(n)) + .1365995*cos(4.*pi*i/(n)) - .0106411*cos(6.*pi*i/(n));
}

TGraph * RUtil::applyWindow(TGraph *inGr, double startt, double endt, int type){
  auto outGr=new TGraph(inGr->GetN());
  int lowInd=RUtil::getIndex(inGr, startt);
  int highInd=RUtil::getIndex(inGr, endt);
  int n=highInd-lowInd;
  for(int i=0;i<inGr->GetN();i++){
    if(i<lowInd||i>highInd){
      outGr->SetPoint(i, inGr->GetX()[i], 0.);
    }
    else{
      outGr->SetPoint(i, inGr->GetX()[i], inGr->GetY()[i]*RUtil::window(i-lowInd, n, type));
    }
  }
  return outGr;
}

TGraph2D * RUtil::applyWindow(TGraph2D *inGr, double startt, double endt, int type){
  auto outGr=new TGraph2D(inGr->GetN());
  int lowInd=RUtil::getIndex(inGr, startt);
  int highInd=RUtil::getIndex(inGr, endt);
  int n=highInd-lowInd;
  for(int i=0;i<inGr->GetN();i++){
    if(i<lowInd||i>highInd){
      outGr->SetPoint(i, inGr->GetX()[i], 0., 0.);
    }
    else{
      outGr->SetPoint(i, inGr->GetX()[i], inGr->GetY()[i]*RUtil::window(i-lowInd, n, type), inGr->GetZ()[i]*RUtil::window(i-lowInd, n, type));
    }
  }
  return outGr;
}

TGraph * RUtil::plotWindow(double peakAmplitude, double len, double GSs, double startt, double endt, int type){
  int N=len*GSs;
  double dt=1./GSs;
  auto outGr=new TGraph(N);
  for(int i=0;i<N;i++){
    outGr->SetPoint(outGr->GetN(), i*dt, peakAmplitude);
  }
  outGr=applyWindow(outGr, startt, endt, type);
  return outGr;
}

double RUtil::deg2Rad(double deg) {
  return (deg * pi / 180.);
}

double RUtil::rad2Deg(double rad) {
  return (rad * 180. / pi);
}

/*************some plotting things****************/

void RUtil::style(TGraph *inGr, Color_t color, double lineWidth, int lineStyle){
  inGr->SetLineColor(color);
  inGr->SetLineWidth(lineWidth);
  inGr->SetLineStyle(lineStyle);
}

void RUtil::style(TH1F *inGr, Color_t color, double lineWidth, int lineStyle){
  inGr->SetLineColor(color);
  inGr->SetLineWidth(lineWidth);
  inGr->SetLineStyle(lineStyle);
}

void RUtil::style(TH1D *inGr, Color_t color, double lineWidth, int lineStyle){
  inGr->SetLineColor(color);
  inGr->SetLineWidth(lineWidth);
  inGr->SetLineStyle(lineStyle);
}

void RUtil::titles(TGraph *inGr, TString title, TString xtitle, TString ytitle){
  auto sizeT=.055;
  inGr->SetTitle(title);

  inGr->GetXaxis()->SetTitle(xtitle);
  inGr->GetYaxis()->SetTitle(ytitle);

  inGr->GetXaxis()->SetTitleSize(sizeT);
  inGr->GetYaxis()->SetTitleSize(sizeT);

  inGr->GetXaxis()->SetLabelSize(sizeT);
  inGr->GetYaxis()->SetLabelSize(sizeT);
  //inGr->GetXaxis()->SetTitleOffset(1.2);
  inGr->GetYaxis()->SetLabelOffset(.01);
  inGr->GetYaxis()->SetTitleOffset(1.2);
}

void RUtil::titles(TF1 *inGr, TString title, TString xtitle, TString ytitle){
  auto sizeT=.055;
  inGr->SetTitle(title);

  inGr->GetXaxis()->SetTitle(xtitle);
  inGr->GetYaxis()->SetTitle(ytitle);

  inGr->GetXaxis()->SetTitleSize(sizeT);
  inGr->GetYaxis()->SetTitleSize(sizeT);

  inGr->GetXaxis()->SetLabelSize(sizeT);
  inGr->GetYaxis()->SetLabelSize(sizeT);
  //inGr->GetXaxis()->SetTitleOffset(1.2);
  inGr->GetYaxis()->SetLabelOffset(.01);
  inGr->GetYaxis()->SetTitleOffset(1.2);
}

void RUtil::ranges(TGraph *inGr,double x1, double x2, double y1, double y2){
  RUtil::yrange(inGr, y1, y2);
  RUtil::xrange(inGr, x1, x2);
}
void RUtil::yrange(TGraph *inGr, double y1, double y2){
  inGr->GetYaxis()->SetRangeUser(y1, y2);
}

void RUtil::xrange(TGraph *inGr, double x1, double x2){
  inGr->GetXaxis()->SetRangeUser(x1, x2);
}

void RUtil::titles(TGraphErrors *inGr, TString title, TString xtitle, TString ytitle){
  inGr->SetTitle(title);
inGr->GetXaxis()->SetTitle(xtitle);
  inGr->GetYaxis()->SetTitle(ytitle);
    inGr->GetXaxis()->SetTitle(xtitle);
  inGr->GetYaxis()->SetTitle(ytitle);
  auto sizeT=.055;
  inGr->GetXaxis()->SetTitleSize(sizeT);
  inGr->GetYaxis()->SetTitleSize(sizeT);

  inGr->GetXaxis()->SetLabelSize(sizeT);
  inGr->GetYaxis()->SetLabelSize(sizeT);
  //inGr->GetXaxis()->SetTitleOffset(1.2);
  inGr->GetYaxis()->SetLabelOffset(.01);
  inGr->GetYaxis()->SetTitleOffset(1.2);
}

void RUtil::ranges(TGraphErrors *inGr,double x1, double x2, double y1, double y2){
  RUtil::yrange(inGr, y1, y2);
  RUtil::xrange(inGr, x1, x2);
}
void RUtil::yrange(TGraphErrors *inGr, double y1, double y2){
  inGr->GetYaxis()->SetRangeUser(y1, y2);
}

void RUtil::xrange(TGraphErrors *inGr, double x1, double x2){
  inGr->GetXaxis()->SetRangeUser(x1, x2);
}

void RUtil::titles(TProfile *inGr, TString title, TString xtitle, TString ytitle){
  inGr->SetTitle(title);
  inGr->GetXaxis()->SetTitle(xtitle);
  inGr->GetYaxis()->SetTitle(ytitle);
    inGr->GetXaxis()->SetTitle(xtitle);
  inGr->GetYaxis()->SetTitle(ytitle);
  auto sizeT=.055;
  inGr->GetXaxis()->SetTitleSize(sizeT);
  inGr->GetYaxis()->SetTitleSize(sizeT);

  inGr->GetXaxis()->SetLabelSize(sizeT);
  inGr->GetYaxis()->SetLabelSize(sizeT);
  //inGr->GetXaxis()->SetTitleOffset(1.2);
  inGr->GetYaxis()->SetLabelOffset(.01);
  inGr->GetYaxis()->SetTitleOffset(1.2);
}

void RUtil::ranges(TProfile *inGr,double x1, double x2, double y1, double y2){
  RUtil::yrange(inGr, y1, y2);
  RUtil::xrange(inGr, x1, x2);
}
void RUtil::yrange(TProfile *inGr, double y1, double y2){
  inGr->GetYaxis()->SetRangeUser(y1, y2);
}

void RUtil::xrange(TProfile *inGr, double x1, double x2){
  inGr->GetXaxis()->SetRangeUser(x1, x2);
}

void RUtil::titles(TH1F *inGr, TString title, TString xtitle, TString ytitle){
  inGr->SetTitle(title);
  inGr->GetXaxis()->SetTitle(xtitle);
  inGr->GetYaxis()->SetTitle(ytitle);
    inGr->GetXaxis()->SetTitle(xtitle);
  inGr->GetYaxis()->SetTitle(ytitle);
  auto sizeT=.055;
  inGr->GetXaxis()->SetTitleSize(sizeT);
  inGr->GetYaxis()->SetTitleSize(sizeT);

  inGr->GetXaxis()->SetLabelSize(sizeT);
  inGr->GetYaxis()->SetLabelSize(sizeT);
  //inGr->GetXaxis()->SetTitleOffset(1.2);
  inGr->GetYaxis()->SetLabelOffset(.01);
  inGr->GetYaxis()->SetTitleOffset(1.2);

}

void RUtil::ranges(TH1F *inGr,double x1, double x2, double y1, double y2){
  RUtil::yrange(inGr, y1, y2);
  RUtil::xrange(inGr, x1, x2);
}
void RUtil::yrange(TH1F *inGr, double y1, double y2){
  inGr->GetYaxis()->SetRangeUser(y1, y2);
}

void RUtil::xrange(TH1F *inGr, double x1, double x2){
  inGr->GetXaxis()->SetRangeUser(x1, x2);
}

void RUtil::titles(TH1D *inGr, TString title, TString xtitle, TString ytitle){
  inGr->SetTitle(title);
  inGr->GetXaxis()->SetTitle(xtitle);
  inGr->GetYaxis()->SetTitle(ytitle);
    inGr->GetXaxis()->SetTitle(xtitle);
  inGr->GetYaxis()->SetTitle(ytitle);
  auto sizeT=.055;
  inGr->GetXaxis()->SetTitleSize(sizeT);
  inGr->GetYaxis()->SetTitleSize(sizeT);

  inGr->GetXaxis()->SetLabelSize(sizeT);
  inGr->GetYaxis()->SetLabelSize(sizeT);
  //inGr->GetXaxis()->SetTitleOffset(1.2);
  inGr->GetYaxis()->SetLabelOffset(.01);
  inGr->GetYaxis()->SetTitleOffset(1.2);

}

void RUtil::ranges(TH1D *inGr,double x1, double x2, double y1, double y2){
  RUtil::yrange(inGr, y1, y2);
  RUtil::xrange(inGr, x1, x2);
}
void RUtil::yrange(TH1D *inGr, double y1, double y2){
  inGr->GetYaxis()->SetRangeUser(y1, y2);
}


void RUtil::xrange(TH1D *inGr, double x1, double x2){
  inGr->GetXaxis()->SetRangeUser(x1, x2);
}

void RUtil::titles(TH2D *inGr, TString title, TString xtitle, TString ytitle, TString ztitle){
  inGr->SetTitle(title);
  inGr->GetXaxis()->SetTitle(xtitle);
  inGr->GetYaxis()->SetTitle(ytitle);
  inGr->GetZaxis()->SetTitle(ztitle);
  inGr->GetZaxis()->SetTitleOffset(1.2);
  inGr->GetXaxis()->SetTitleSize(.05);
  inGr->GetYaxis()->SetTitleSize(.05);
  inGr->GetZaxis()->SetTitleSize(.05);
  inGr->GetXaxis()->SetLabelSize(.05);
  inGr->GetYaxis()->SetLabelSize(.05);
  inGr->GetZaxis()->SetLabelSize(.05);
  inGr->GetYaxis()->SetTitleOffset(1.);
}

void RUtil::ranges(TH2D *inGr,double x1, double x2, double y1, double y2, double z1, double z2){
  RUtil::yrange(inGr, y1, y2);
  RUtil::xrange(inGr, x1, x2);
  if(z1!=0&&z2!=0){
    RUtil::zrange(inGr, z1, z2);
  }

}
void RUtil::yrange(TH2D *inGr, double y1, double y2){
  inGr->GetYaxis()->SetRangeUser(y1, y2);
}

void RUtil::xrange(TH2D *inGr, double x1, double x2){
  inGr->GetXaxis()->SetRangeUser(x1, x2);
}
void RUtil::zrange(TH2D *inGr, double z1, double z2){
  inGr->GetZaxis()->SetRangeUser(z1, z2);
}


void RUtil::titles(TH2F *inGr, TString title, TString xtitle, TString ytitle, TString ztitle){
  inGr->SetTitle(title);
  inGr->GetXaxis()->SetTitle(xtitle);
  inGr->GetYaxis()->SetTitle(ytitle);
  inGr->GetZaxis()->SetTitle(ztitle);
  inGr->GetZaxis()->SetTitleOffset(1.1);
  inGr->GetXaxis()->SetTitleSize(.05);
  inGr->GetYaxis()->SetTitleSize(.05);
  inGr->GetZaxis()->SetTitleSize(.05);
  inGr->GetXaxis()->SetLabelSize(.05);
  inGr->GetYaxis()->SetLabelSize(.05);
  inGr->GetZaxis()->SetLabelSize(.05);
  inGr->GetYaxis()->SetTitleOffset(1.);
}
void RUtil::ranges(TH2F *inGr,double x1, double x2, double y1, double y2){
  RUtil::yrange(inGr, y1, y2);
  RUtil::xrange(inGr, x1, x2);
}
void RUtil::yrange(TH2F *inGr, double y1, double y2){
  inGr->GetYaxis()->SetRangeUser(y1, y2);
}

void RUtil::xrange(TH2F *inGr, double x1, double x2){
  inGr->GetXaxis()->SetRangeUser(x1, x2);
}

void RUtil::draw(vector<TGraph*> inGr, TString option){
  if(option=="norm"){
    RUtil::normalize(inGr[0])->Draw("al PLC");
  }
  else{
    inGr[0]->Draw("al PLC");
  }
  for(int i=1;i<inGr.size();i++){
    inGr[i]->SetLineStyle(fmod(i, 9)+1);
    if(option=="norm"){
      RUtil::normalize(inGr[i])->Draw("l same PLC");
    }
    else{
      inGr[i]->Draw("l same PLC");
    }
    
  }
}


void RUtil::draw(vector<TH1D*> inGr, TString drawOption1){
  inGr[0]->Draw(drawOption1);

  for(int i=0;i<inGr.size();i++){
    inGr[i]->Draw(drawOption1+" same");
    
  }
}


void RUtil::draw(int nGraphs, TGraph** inGr, TString option){
  if(option=="norm"){
    RUtil::normalize(inGr[0])->Draw("al PLC");
  }
  else{
    inGr[0]->Draw("al PLC");
  }
  for(int i=0;i<nGraphs;i++){
    if(option=="norm"){
      RUtil::normalize(inGr[i])->Draw("l same PLC");
    }
    else{
      inGr[i]->Draw("l same PLC");
    }
    
  }
}


  


TGraph * RUtil::toGraph(TH1D * hist){
  auto   outGr=new TGraph();
  for(int i=0;i<hist->GetNbinsX();i++){
    outGr->SetPoint(outGr->GetN(), hist->GetBinCenter(i), hist->GetBinContent(i));
  }
  return outGr;
}

TGraph * RUtil::toGraph(TH1F * hist){
  auto   outGr=new TGraph();
  for(int i=0;i<hist->GetNbinsX();i++){
    outGr->SetPoint(outGr->GetN(), hist->GetBinCenter(i), hist->GetBinContent(i));
  }
  return outGr;
}

TGraph * RUtil::getSliceY(TH2D* hist, double ylow, double yhigh){
  int binx, binylow, binyhigh, binz;
  hist->GetBinXYZ(hist->FindBin(0, ylow), binx, binylow, binz);
  hist->GetBinXYZ(hist->FindBin(0, yhigh), binx, binyhigh, binz);
  return RUtil::toGraph(hist->ProjectionX(TString::Itoa(ylow, 10), binylow, binyhigh));
}

vector<TGraph*> RUtil::getSlicesY(TH2D * hist, int nSlices, double ylow, double yhigh, TString name, TString title){
  double dy=(yhigh-ylow)/(double)nSlices;
  auto out=vector<TGraph*>();
  for(int i=0;i<nSlices;i++){
    auto y1=ylow+(double)i*dy;
    auto y2=y1+dy;
    auto gr=RUtil::getSliceY(hist,y1, y2);
    gr->SetName(Form("%.2f", (y2+y1)/2.));
    gr->SetTitle(Form("%.2f", (y2+y1)/2.));
    out.push_back(gr);
  }
  return out;
}

vector<TLine*> RUtil::drawPeakCursorXY(TH2D* inHist, Color_t color){
  auto binx=0, biny=0, binz=0;
  auto maxbin=inHist->GetMaximumBin(binx, biny, binz);
  auto x = inHist->GetXaxis()->GetBinCenter(binx);
  auto y = inHist->GetYaxis()->GetBinCenter(biny);
  auto maxx=gPad->GetUxmax();
  auto minx=gPad->GetUxmin();
  auto maxy=gPad->GetUymax();
  auto miny=gPad->GetUymin();
  auto xline=new TLine(x, miny, x, maxy);
  xline->SetLineColor(color);
  auto yline=new TLine(minx, y, maxx,y);
  yline->SetLineColor(color);
  xline->Draw();
  yline->Draw();
  auto outlines=vector<TLine*>(2);
  outlines[0]=xline;
  outlines[1]=yline;
  return outlines;
}

// void RUtil::draw(TGraph** inGr){
//   inGr[0]->Draw("al PLC");
//   for(int i=0;i<inGr.size();i++){
//     inGr[i]->Draw("l same PLC");
//   }
// }




void RUtil::setWarmPalette(double alpha){

  const Int_t rgb = 3;
  const Int_t N = 255;

  Double_t stops[rgb] = {0.34, 0.61, 0.84};
  Double_t red[rgb]   = {0.00, 1., 0.90};
  Double_t green[rgb] = {0., 0.0, 1.0};
  Double_t blue[rgb]  = {0.00, 0.0, 0.00};
  TColor::CreateGradientColorTable(rgb, stops, red, green, blue, N, alpha);
  gStyle->SetNumberContours(N);

}

void RUtil::setCoolPalette(double alpha){

  const Int_t rgb = 3;
  const Int_t N = 255;


  Double_t red[]    = {0., .0, .0, 1., 1.0};
  Double_t green[]  = {0., .1, .9, .0, 1.0};
  Double_t blue[]   = {0., .80, .90, 0.20, 1.0};
  Double_t stops[] = {0., .25, .50, .75, 1.0};
  TColor::CreateGradientColorTable(rgb, stops, red, green, blue, N, alpha);
  gStyle->SetNumberContours(N);

}

TCanvas * RUtil::canvas(TString title, TString name, int xdim, int ydim){
  auto can=new TCanvas(title, name, xdim, ydim);
  can->SetLeftMargin(.15);
  can->SetBottomMargin(.12);
  can->SetRightMargin(.18);
  return can;
}
void RUtil::setColdPalette(double alpha){

  const Int_t rgb = 3;
  const Int_t N = 255;

  double red[]=   { 0.00, 0.00, 0.00, 1.00, 1.00};
  double green[]= { 0.00, 0.00, 0.00, 0.80, 1.00};
  double blue[]=  { 0.00, 0.40, 0.80, 0.90, 1.00};
  double stops[]={ 0.00, 0.30, 0.50, 0.90, 1.00};

  TColor::CreateGradientColorTable(5, stops, red, green, blue, N, alpha);
  gStyle->SetNumberContours(N);

}

void RUtil::setHotPalette(double alpha){

  const Int_t rgb = 5;
  const Int_t N = 255;
  
  double red[]=   {0.00, 0.50, 0.70, 0.90, 1.00};
  double green[]= {0.00, 0.00, 0.00, 0.50, 1.00};
  double blue[]=  {0.00, 0.10, 0.00, 0.00, 1.00};
  double stops[]={0.00, 0.35, 0.60, 0.85, 1.00};

  TColor::CreateGradientColorTable(rgb, stops, red, green, blue, N, alpha);
  gStyle->SetNumberContours(N);

}

void RUtil::set2DPalette(){
  const Int_t rgb = 5;
  const Int_t N = 255;
  
  double red[]=     {0.90, 0.70, 1.0, 0.00, 0.00};
  double green[]= {0.00, 0.40, 1.0, 0.40, 0.00};
  double blue[]=  {0.00, 0.00, 1.00, 0.70, 0.90};
  double stops[]={0.00, 0.25, 0.5-((double)1./254), 0.75, 1.00};

  TColor::CreateGradientColorTable(rgb, stops, red, green, blue, N);
  gStyle->SetNumberContours(N);

}

TGraph * RUtil::evenSample(TGraph *inGr, double dt){
  inGr->SetBit(TGraph::kIsSortedX);
  double maxT=inGr->GetX()[inGr->GetN()-1];
  double minT=inGr->GetX()[0];

  double t=minT;
  auto xx=vector<double>();
  auto yy=vector<double>();
  while(t<=maxT){
    xx.push_back(t);
    yy.push_back(inGr->Eval(t));
    t+=dt;
  }
  TGraph * outGr=new TGraph(xx.size(), &xx[0], &yy[0]);
  return outGr;
    
}



TGraph * RUtil::zeroPad(TGraph *inGr, int num, int whichEnd){
  auto dt=inGr->GetX()[1]-inGr->GetX()[0];
  auto outGr=(TGraph*)inGr->Clone();
  auto lastt=inGr->GetX()[inGr->GetN()-1];
  if(whichEnd==1){
    for(int i=0;i<num;i++){
      outGr->SetPoint(outGr->GetN(), lastt+((double)i*dt), 0.);
    }
  }
  else {
    auto tempGr=new TGraph(inGr->GetN()+num);
    auto firstt=inGr->GetX()[0];
    for(int i=tempGr->GetN()-1;i>=num;i--){
      tempGr->SetPoint(i, outGr->GetX()[i-num], outGr->GetY()[i-num]);
    }
    for(int i=num-1;i>=0;i--){
      tempGr->SetPoint(i, firstt-((double)i*dt), 0.);
    }
    return tempGr;
  }
  return outGr;
}

TGraph2D * RUtil::zeroPad(TGraph2D *inGr, int num, int whichEnd){
  auto dt=inGr->GetX()[1]-inGr->GetX()[0];
  auto outGr=(TGraph2D*)inGr->Clone();
  auto lastt=inGr->GetX()[inGr->GetN()-1];
  if(whichEnd==1){
    for(int i=0;i<num;i++){
      outGr->SetPoint(outGr->GetN(), lastt+((double)i*dt), 0.,0.);
    }
  }
  else {
    auto tempGr=new TGraph2D(inGr->GetN()+num);
    auto firstt=inGr->GetX()[0];
    auto t0=firstt-(num*dt);
    for(int i=num;i<tempGr->GetN();i++){
      tempGr->SetPoint(i, outGr->GetX()[i-num], outGr->GetY()[i-num], outGr->GetZ()[i-num]);
    }
    for(int i=0;i<num;i++){
      tempGr->SetPoint(i, t0+(double)i*dt, 0.,0.);
    }
    return tempGr;
  }
  return outGr;
}

double RUtil::distance3(TVector3 one, TVector3 two){
  return abs((one-two).Mag());
}

double * RUtil::distance3(int N, TVector3 one, TVector3 *two){
  double *out=(double*)malloc(sizeof(double) * N);
  for(int i=0;i<N;i++){
    out[i]=RUtil::distance3(one, two[i]);
  }
  return out;
}
  
double RUtil::timeOfFlight(TVector3 one, TVector3 two, double n){
  return RUtil::distance3(one, two)*n/RUtil::c_light;
}

double * RUtil::timeOfFlight(int N, TVector3 one, TVector3 *two, double n){
  double *out=(double*)malloc(sizeof(double) * N);
  for(int i=0;i<N;i++){
    out[i]=RUtil::timeOfFlight(one, two[i]);
  }
  return out;
}

double  RUtil::dTimeOfFlight(TVector3 source,TVector3 one, TVector3 two, double n){
  auto time1=RUtil::timeOfFlight(source,one, n);
  auto time2=RUtil::timeOfFlight(source,two, n);

  return time1-time2;
}

double ** RUtil::dTimeOfFlight(int N, TVector3 one, TVector3 *two, double n){
  auto times=RUtil::timeOfFlight(N, one, two, n);
  double ** out = new double*[N];
  for(int i=0;i<N;i++){
    out[i]=new double[N];
    for(int j=0;j<N;j++){
      out[i][j]=times[i]-times[j];
    }
  }
  return out;
}


double RUtil::dBToLinear(double dB){
  return pow(10, dB/10.);
}

double RUtil::linearToDBPower(double linear){
  return 10.*log10(linear);
}


double RUtil::lInt(double logE){
  if(logE>=1&&logE<4){
    return 14.126+(-.939*logE);
  }
  if(logE>=4&&logE<5){
    return 12.943+(-.636*logE);
  }
  if(logE>=5&&logE<7){
    return 12.19+(-.489*logE);
  }
  if(logE>=7&&logE<12){
    return 11.31+(-.3642*logE);
  }
  return 0;
}


double RUtil::SIM::ss(double x, double E, double x_0, double e_0){
  return (3.*x/x_0)/((x/x_0)+2.*log(E/e_0));
}


double RUtil::SIM::n(double x, double E, double x_0, double e_0){
  return (.31 * exp((x/x_0)*(1-1.5*log(RUtil::SIM::ss(x, E, x_0, e_0)))))/sqrt(log(E/e_0));
}



  

//unevenly sampled (in time) dft
TGraph2D * RUtil::DFT::udft(TGraph * inGr, double fSampMean){
  int N=inGr->GetN();
  double fStep=(fSampMean/(double)N);
  double fudge=0.*(fStep/1000.)*(((double)rand()/RAND_MAX));
  auto xx=inGr->GetX();

  auto sinT=vector<vector<double>>(N, vector<double>(N, 0.));
  auto cosT=vector<vector<double>>(N, vector<double>(N, 0.)); 

  //auto S=vector<double>(N, 0.);
  //auto C=vector<double>(N, 0.);

  
  for(int f=0;f<N;f++){
    for(int t=0;t<N;t++){
      sinT[f][t]=2.*sin(fStep*2.*RUtil::pi*inGr->GetX()[t]*f)/N;
      cosT[f][t]=2.*cos(fStep*2.*RUtil::pi*inGr->GetX()[t]*f)/N;     
    }
    //    cout<<f*fStep<<endl;
  }

  

  TGraph2D *out=new TGraph2D();

  for(int f=0;f<N;f++){
    double S=0.;
    double C=0.;
    for(int t=0;t<N;t++){
      S+=sinT[f][t]*inGr->GetY()[t];
      C+=cosT[f][t]*inGr->GetY()[t];
    }
    out->SetPoint(f, fStep*f, S, C);
  }
  //TGraph2D *out=new TGraph2D();
  // auto normGr=RUtil::normalize(inGr);
  // //auto amp=RUtil::amplitude(inGr, 0, 20);
  // out->SetPoint(out->GetN(), 0,0,0);
  // for(int f=1;f<N;f++){
  //   //auto val=0;
  //   auto cwSin=RUtil::sampledCW(f*fStep, 1., N, xx, 0.);
  //   auto cwCos=RUtil::sampledCW(f*fStep, 1., N, xx, RUtil::pi/2.);
  //   auto S=RUtil::dot(normGr, cwSin);
  //   auto C=RUtil::dot(normGr, cwCos);
  //   //    cout<<xx[0]<<" "<<cwSin->GetY()[0]<<" "<<cwCos->GetY()[0]<<" "<<S<<" "<<C<<endl;
  //   //if(inGr->GetN()!=cwCos->GetN())cout<<cwCos->GetN();
  //   out->SetPoint(out->GetN(), fStep*f, S, C);
  //   delete cwSin;
  //   delete cwCos;
  // }
  return out;
}


TGraph * RUtil::DFT::iudft(TGraph2D * inGr, double GSs){
  int N=inGr->GetN();
  int Nt=N*(GSs/inGr->GetX()[N-1]);
  double dt=1./GSs;

  double prefactor=1.;///(4.*RUtil::pi);
  auto  vals=vector<double>(Nt, 0.);
  for(int f=0;f<N/2+1;f++){
    for(int t=0;t<Nt;t++){
      double T=(double)t*dt;
      vals[t]+=prefactor*inGr->GetY()[f]*sin(2.*RUtil::pi*inGr->GetX()[f]*T);
      vals[t]+=prefactor*inGr->GetZ()[f]*cos(2.*RUtil::pi*inGr->GetX()[f]*T);
    }
  }
  auto ind=RUtil::makeIndices(Nt, dt);
  TGraph *out=new TGraph(Nt, ind, &vals[0]);
  delete ind;
  return out;
}



//unevenly sampled (in time) psd
TGraph * RUtil::DFT::upsd(TGraph * inGr, double fSampMean, int log){
  int N=inGr->GetN();
  double fStep=(fSampMean/(double)N);
  double fudge=0.*(fStep/100.)*(double)rand()/RAND_MAX;
  auto xx=inGr->GetX();

  auto sinT=vector<vector<double>>(N, vector<double>(N, 0.));
  auto cosT=vector<vector<double>>(N, vector<double>(N, 0.)); 

  //auto S=vector<double>(N, 0.);
  //auto C=vector<double>(N, 0.);

  
  for(int f=0;f<N;f++){
    for(int t=0;t<N;t++){
      sinT[f][t]=2.*sin((fStep+fudge)*2.*RUtil::pi*inGr->GetX()[t]*f)/N;
      cosT[f][t]=2.*cos((fStep+fudge)*2.*RUtil::pi*inGr->GetX()[t]*f)/N;     
    }
    //    cout<<f*fStep<<endl;
  }

  

  TGraph *out=new TGraph();

  for(int f=0;f<N;f++){
    double S=0.;
    double C=0.;
    for(int t=0;t<N;t++){
      S+=sinT[f][t]*inGr->GetY()[t];
      C+=cosT[f][t]*inGr->GetY()[t];
    }
    if(log==1){
      out->SetPoint(f, fStep*f, RUtil::vToDbmHz(fSampMean/2.,sqrt(S*S+C*C)));
    }
    else{
      out->SetPoint(f, fStep*f,  sqrt(S*S+C*C));
    }
  }
  //TGraph2D *out=new TGraph2D();
  // auto normGr=RUtil::normalize(inGr);
  // //auto amp=RUtil::amplitude(inGr, 0, 20);
  // out->SetPoint(out->GetN(), 0,0,0);
  // for(int f=1;f<N;f++){
  //   //auto val=0;
  //   auto cwSin=RUtil::sampledCW(f*fStep, 1., N, xx, 0.);
  //   auto cwCos=RUtil::sampledCW(f*fStep, 1., N, xx, RUtil::pi/2.);
  //   auto S=RUtil::dot(normGr, cwSin);
  //   auto C=RUtil::dot(normGr, cwCos);
  //   //    cout<<xx[0]<<" "<<cwSin->GetY()[0]<<" "<<cwCos->GetY()[0]<<" "<<S<<" "<<C<<endl;
  //   //if(inGr->GetN()!=cwCos->GetN())cout<<cwCos->GetN();
  //   out->SetPoint(out->GetN(), fStep*f, S, C);
  //   delete cwSin;
  //   delete cwCos;
  // }
  return out;
}


void RUtil::demod::normalizedDeChirp(Int_t * one, Int_t * two, int offset, int insize, Float_t * out){
  int size = insize-offset-offset;
  //  double out[size];
  double value, denom, x, y;

  for(int i=0;i<size;i++){
    x+=one[i];
    y+=two[i];
  }
  x=pow(x, 2)/(double)size;
  y=pow(y, 2)/(double)size;
  denom = sqrt(x*y);

  for(int i=0;i<insize;i++){
    if(i+offset < insize){//sanity check
      value = ((Float_t)one[i]*(Float_t)two[i+offset])/denom;
      out[i]=value; 
    }
  }
  //	return out;
}


TGraph * RUtil::demod::normalizedDeChirp(TGraph * one, int offset){
  int insize=one->GetN();//>two->GetN()?two->GetN():one->GetN();
  int size = insize-offset-offset;
  //  double out[size];
  double value, denom, x, y;

  for(int i=0;i<size;i++){
    x+=one->GetY()[i];
    //y+=two->GetY()[i];
  }
  //x=pow(x, 2)/(double)size;
  //y=pow(y, 2)/(double)size;
  //denom = sqrt(x*y);
  denom=pow(x, 2)/(double)size;
  
  auto outgr=new TGraph();
  for(int i=0;i<insize;i++){
    if(i+offset < insize){//sanity check
      value = (one->GetY()[i]*one->GetY()[i+offset])/denom;
      outgr->SetPoint(outgr->GetN(), one->GetX()[i], value);
    }
  }
  return outgr;
}

TGraph * RUtil::demod::deChirp(TGraph * one, int offset){
  int insize=one->GetN();//>two->GetN()?two->GetN():one->GetN();
  int size = insize-offset-offset;
  //  double out[size];
  double value, denom, x, y;

  // for(int i=0;i<size;i++){
  //   x+=one->GetY()[i];
  //   //y+=two->GetY()[i];
  // }
  // //x=pow(x, 2)/(double)size;
  // //y=pow(y, 2)/(double)size;
  // //denom = sqrt(x*y);
  // denom=pow(x, 2)/(double)size;
  
  auto outgr=new TGraph();
  for(int i=0;i<insize;i++){
    if(i+offset < insize){//sanity check
      value = (one->GetY()[i]*one->GetY()[i+offset]);
      outgr->SetPoint(outgr->GetN(), one->GetX()[i], value);
    }
  }
  return outgr;
}


TGraph * RUtil::demod::envelopeDetectorFritschEQ14(TGraph * ingr, double fc, int N){
  double fs=ingr->GetX()[1]-ingr->GetX()[0];
  double omega0=2*RUtil::pi*fc/fs;
  auto outgr=new TGraph(ingr->GetN()-N);

  for(int i=0;i<ingr->GetN()-N;i++){
    outgr->SetPoint(i, ingr->GetX()[i], sqrt(pow(ingr->GetY()[i+N], 2)+ pow((ingr->GetY()[i]/sin(omega0)) - (ingr->GetY()[i+N]/tan(omega0)),2)));
  }
  return outgr;
    
}





TGraph * RUtil::demod::envelopeDetectorFritschEQ11(TGraph * ingr, double fc){
  double fs=ingr->GetX()[1]-ingr->GetX()[0];
  double omega0=2*RUtil::pi*fc/fs;
  auto outgr=new TGraph(ingr->GetN()-1);
  auto amp=1./sin(omega0);
  for(int i=0;i<ingr->GetN()-1;i++){
    outgr->SetPoint(i, ingr->GetX()[i], amp*sqrt(pow(ingr->GetY()[i], 2)+ pow(ingr->GetY()[i+1], 2)-2*ingr->GetY()[i]*ingr->GetY()[i+1]*cos(omega0)));
  }
  return outgr;
    
}


TH2D * RUtil::wavelet::simpleWaveletTransform(TGraph *gr1, int order, int width){
  auto len=gr1->GetN();
  auto dt=gr1->GetX()[1]-gr1->GetX()[0];
  auto Fs=1./dt;
  auto og=new TH2D("wavelet", "wavelet", len, gr1->GetX()[0], gr1->GetX()[len-1],len/2-1, 0, Fs/2);


  auto centerF=RUtil::linspace(0,Fs/2., order);
  // auto wavelet=new TGraph();
  //auto conv=new TGraph();

  for(int i=1;i<order;i++){
  
    auto wavelet=RUtil::wavelet::morlet(centerF[i], len, width, dt);
    //    cout<<centerF[i]<<endl;
    auto conv=RUtil::FFT::convolve(gr1,wavelet);
    auto waveletFFT=(TGraph2D*)fXfrmGr2D->Clone();
    waveletFFT->SetName("waveletFFT");
    auto hilbb=RUtil::FFT::hilbertEnvelope(conv);
    for(int j=0;j<len;j++){
      for(int k=0;k<len/2-1;k++){
	auto zz=waveletFFT->GetZ()[k];
	auto yy=waveletFFT->GetY()[k];
	og->Fill(conv->GetX()[j], 2.*waveletFFT->GetX()[k],abs(hilbb->GetY()[j])*sqrt(zz*zz+yy*yy));
	//auto val=hilbb->GetY()[k];
	//og->Fill(conv->GetX()[j], conv->GetX()[k],val);
      
      // og->SetBinContent(j,i,abs(conv->GetY()[j])*sqrt(fXfrmGr2D->GetY()[j]*fXfrmGr2D->GetY()[j] + fXfrmGr2D->GetZ()[j]*fXfrmGr2D->GetZ()[j]));
      }
    }
     delete conv;
     delete waveletFFT;
     delete hilbb;
     delete wavelet;
  }
  
  return og;
}

TH2D * RUtil::wavelet::simpleSuperletTransform(TGraph *gr1, int order, int width, int super){
  auto len=gr1->GetN();
  auto dt=gr1->GetX()[1]-gr1->GetX()[0];
  auto Fs=1./dt;
  auto og=new TH2D("superlet", "superlet", len, gr1->GetX()[0], gr1->GetX()[len-1],len/2-1, 0, Fs/2);


  auto centerF=RUtil::linspace(0,Fs/2., order);

  auto superW=RUtil::linspace(width/(super-1), width*(super-1), super);

  for(int i=1;i<order;i++){
    for(int l=0;l<super;l++){
      auto wavelet=RUtil::wavelet::morlet(centerF[i], len, superW[l], dt);

      auto conv=RUtil::FFT::convolve(gr1,wavelet);
      auto waveletFFT=(TGraph2D*)fXfrmGr2D->Clone();
      waveletFFT->SetName("waveletFFT");
      auto hilbb=RUtil::FFT::hilbertEnvelope(conv);
      for(int j=0;j<len;j++){
	for(int k=0;k<len/2-1;k++){
	  auto zz=waveletFFT->GetZ()[k];
	  auto yy=waveletFFT->GetY()[k];
	  auto binx=og->GetXaxis()->FindBin(conv->GetX()[j]);
	  auto biny=og->GetYaxis()->FindBin(2.*waveletFFT->GetX()[k]);
	  auto binxy=og->FindBin(conv->GetX()[j],2.*waveletFFT->GetX()[k]);
	  auto temp=og->GetBinContent(binx,biny);
	  auto currVal=temp>0?temp:1.;
	  og->Fill(conv->GetX()[j], 2.*waveletFFT->GetX()[k], abs(hilbb->GetY()[j])*sqrt(zz*zz+yy*yy));
	  //og->SetBinContent(binx,biny, ((hilbb->GetY()[j])*sqrt(zz*zz+yy*yy)+currVal));
	}
      }
      delete conv;
      delete waveletFFT;
      delete hilbb;
      delete wavelet;
    }
  }
  return og;
}

TGraph* RUtil::wavelet::morlet(double centerF, int length, int width, double dt){
  auto  cw=RUtil::makeCW(centerF, 1., 0, length, 1./dt, RUtil::pi/2);
  auto centerT=cw->GetX()[cw->GetN()/2];
  auto windowedCW=RUtil::applyWindow(cw, centerT-width, centerT+width, 3);
  delete cw;
  return windowedCW;
}
