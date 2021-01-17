#include "ns3/core-module.h"

#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <cerrno>
#include <limits>
#include <cstdlib>
#include <unistd.h>

#include "lte-reporter.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LteReporter");

NS_OBJECT_ENSURE_REGISTERED (LteReporter);


TypeId
LteReporter::GetTypeId (void)
{
  // NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_FUNCTION ("LteReporter::GetTypeId");

  static TypeId tid = TypeId ("ns3::LteReporter")
    .SetParent<Object> ()
    .SetGroupName ("EpcGroup")
    .AddConstructor<LteReporter>()
  ;
  return tid;
}

TypeId
LteReporter::GetInstanceTypeId () const
{
  return GetTypeId ();
}

LteReporter::LteReporter () 
{

}

LteReporter::~LteReporter ()
{

}


uint16_t
LteReporter::runCurl(std::string cmd)
{
  uint16_t status = -1;
  if(!vfork()){
    // std::cout<<"runCurl:"<<cmd<<std::endl;
    status = ::execlp ("curl","curl","-s",cmd.c_str(), (char *)NULL);
  }
  return status;
};

uint16_t
LteReporter::sshDocker(std::string serviceName, std::string path)
{
  uint16_t status = -1;
  if(!vfork()){
    status = ::execlp ("docker","docker","exec","-d",serviceName.c_str(), "curl" ,path.c_str(), (char *)NULL);
  }
  return status;
};

std::string 
LteReporter::exec(std::string command) {
   char buffer[128];
   std::string result = "";
   // std::cout<<"exec : "<<command<<std::endl;
   // Open pipe to file
   FILE* pipe = popen(command.c_str(), "r");
   if (!pipe) {
      return "popen failed!";
   }

   // read till end of process:
   while (!feof(pipe)) {

      // use buffer to read and add to result
      if (fgets(buffer, 128, pipe) != NULL)
         result += buffer;
   }

   pclose(pipe);
   return result;
}

uint16_t
LteReporter::routing(std::string serviceName, uint16_t index)
{
  index = index + 1;
  std::cout<<"serviceName:"<<serviceName<<" "<<index<<std::endl;
  // if(!vfork()){
  //   status = ::execlp ("./usr/ns-allinone-3.30/ns-3.30/scratch/test/script/routingFlex.sh",
  //     serviceName.c_str(),index, (char *)NULL);
  // }
  std::string rs = exec(std::string("/usr/ns-allinone-3.30/ns-3.30/scratch/test/script/routingFlex.sh ")
    + serviceName + std::string(" ") + std::to_string(index));
  std::cout<<rs<<std::endl;
  return 1;
};

void
LteReporter::setEnableReport(bool ck)
{ 
  enableReport = ck;
};


} // namespace ns3