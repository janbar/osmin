#ifndef REMOTE_H
#define REMOTE_H

#include "servicefrontend.h"

class Remote
{
public:
  virtual void connectToService(QVariant service) = 0;
  virtual void connectToService(ServiceFrontendPtr& service) = 0;
};

#endif // REMOTE_H
