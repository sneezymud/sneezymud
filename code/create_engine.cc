#include "stdsneezy.h"
#include "create_engine.h"

int appliedSubstanceCreateBalm  (TBeing *, cmdTypeT, int, TObj *),
    appliedSubstanceCreateSalve (TBeing *, cmdTypeT, int, TObj *),
    appliedSubstanceCreatePowder(TBeing *, cmdTypeT, int, TObj *),
    appliedSubstanceCreateOil   (TBeing *, cmdTypeT, int, TObj *),
    appliedSubstanceCreateIchor (TBeing *, cmdTypeT, int, TObj *);

bool appliedSubstanceCheckBalm  (TBeing *),
     appliedSubstanceCheckSalve (TBeing *),
     appliedSubstanceCheckPowder(TBeing *),
     appliedSubstanceCheckOil   (TBeing *),
     appliedSubstanceCheckIchor (TBeing *);

void StartCreateEngine(CreateEngineMethods, int, TBeing *, TObj *, const char *, TThing **);

int  appliedSubstanceCountList(const char *, char ** = NULL, int = -1);
bool appliedSubstanceCheckList(TBeing *, const char *, char **, TThing **, int),
     appliedSubstanceFindMatch(TThing **, int, int, int);

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

CreateEngineData *AppliedCreate[MAX_APPLIED_SUBSTANCES];

CreateEngineData::CreateEngineData(
 ush_int neClass, int nceType, sh_int nskCEMin, sh_int nskCEMax, sh_int nskCERounds,
 spellNumT nskCESkillNum, const char * nceName, const char * nceStartMsg, const char * nceEndMsg,
 const char * nceMessageA, const char * nceMessageB, const char * nceMessageC, const char * nceMessageD,
 const char * nceMessageE,
 long CompA, long CompB, long CompC, long CompD, long CompE,
 long CompF, long CompG, long CompH, long CompI, long CompJ, long CompK,
 int (*nceStartup)(int, TBeing *, TObj *),
 int (*ncePerMesg)(int, TBeing *, TObj *),
 int (*nceEnding)(int, TBeing *, TObj *)) :
  eClass(neClass),
  ceType(nceType),
  skCEMin(nskCEMin),
  skCEMax(nskCEMax),
  skCERounds(nskCERounds),
  skCESkillNum(nskCESkillNum),
  ceStartup(nceStartup),
  cePerMesg(ncePerMesg),
  ceEnding(nceEnding)
{
  ceName = mud_str_dup(nceName);
  ceMessages[0] = mud_str_dup(nceMessageA);
  ceMessages[1] = mud_str_dup(nceMessageB);
  ceMessages[2] = mud_str_dup(nceMessageC);
  ceMessages[3] = mud_str_dup(nceMessageD);
  ceMessages[4] = mud_str_dup(nceMessageE);

  CompReward = CompA;

  CompList[0] = CompB;
  CompList[1] = CompC;
  CompList[2] = CompD;
  CompList[3] = CompE;
  CompList[4] = CompF;
  CompList[5] = CompG;
  CompList[6] = CompH;
  CompList[7] = CompI;
  CompList[8] = CompJ;
  CompList[9] = CompK;
}

CreateEngineData::~CreateEngineData()
{
  delete [] ceName;
  ceName = NULL;
  delete [] ceMessages[0];
  ceMessages[0] = NULL;
  delete [] ceMessages[1];
  ceMessages[1] = NULL;
  delete [] ceMessages[2];
  ceMessages[2] = NULL;
  delete [] ceMessages[3];
  ceMessages[3] = NULL;
  delete [] ceMessages[4];
  ceMessages[4] = NULL;
}

void SetupCreateEngineData()
{
  memset(AppliedCreate, 0, sizeof(AppliedCreate));

  AppliedCreate[0] = new CreateEngineData(CLASS_RANGER, CEMH_AS_BALM, 0, 100, 5, SKILL_APPLY_HERBS,
    "healing balm", // Name
    "", // StartMessage
    "", // EndMessage
    "", // Message1
    "", // Message2
    "", // Message3
    "", // Message4
    "", // Message5
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL);
  // First 5 substances:
  // 0 : 'common' balm
  // 1 : 'common' salve
  // 2 : 'common' powder
  // 3 : 'common' oil
  // 4 : 'common' ichor
}

// Syntax: create <balm/salve/powder/oil/ichor> <list>
int TBeing::doCreate(const char *tArg)
{
  if (!hasWizPower(POWER_WIZARD)) {
    sendTo("Prototype command.  You need to be a developer to use this.\n\r");
    return 0;
  }

  if (strcmp(getName(), "Lapsos")) {
    sendTo("This code is not only untested but is considered unstable.\n\r");
    return 0;
  }

  int   nRc     =  0,
        ceLevel = -1,
        skLevel =  0,
        ceType  =  0;
  char  chCeType[256],
        chCeList[256],
       *chCeBrLt[10];
  bool  dCrError = false;
  TThing *tObjList[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

  if (hasClass(CLASS_RANGER)) {
    if ((skLevel = getSkillValue(SKILL_APPLY_HERBS)) <= 0)
      sendTo("I'm afraid you don't even know how to apply them, let alone Make them.\n\r");

    if (!tArg || !*tArg)
      dCrError = true;
    else {
      for (; isspace(*tArg); tArg++);

      if (!*tArg)
        dCrError = true;
      else {
        tArg = one_argument(tArg, chCeType);

             if (is_abbrev("balm"  , chCeType))
          ceType = CEMH_AS_BALM;
        else if (is_abbrev("salve" , chCeType))
          ceType = CEMH_AS_SALVE;
        else if (is_abbrev("powder", chCeType))
          ceType = CEMH_AS_POWDER;
        else if (is_abbrev("oil"   , chCeType))
          ceType = CEMH_AS_OIL;
        else if (is_abbrev("ichor" , chCeType))
          ceType = CEMH_AS_ICHOR;
        else
          dCrError = true;

        for (; isspace(*tArg); tArg++);

        if (!*tArg || strlen(tArg) > 255)
          dCrError = true;
        else
          strcpy(chCeList, tArg);
      }
    }

    if (dCrError) {
      sendTo("Syntax: create <balm/salve/powder/oil/ichor> <item-list>\n\r");
      return 0;
    }

    if ((ceLevel == CEMH_AS_BALM   && !appliedSubstanceCheckBalm  (this)) ||
        (ceLevel == CEMH_AS_SALVE  && !appliedSubstanceCheckSalve (this)) ||
        (ceLevel == CEMH_AS_POWDER && !appliedSubstanceCheckPowder(this)) ||
        (ceLevel == CEMH_AS_OIL    && !appliedSubstanceCheckOil   (this)) ||
        (ceLevel == CEMH_AS_ICHOR  && !appliedSubstanceCheckIchor (this))) {
      sendTo("I'm afraid you don't have the items to make that type of substance.\n\r");
      return 0;
    }

    if (appliedSubstanceCountList(chCeList, chCeBrLt, 10) > 10) {
      sendTo("I'm afraid you cannot mix more than 10 items together.\n\r");
      return 0;
    }

    if (!appliedSubstanceCheckList(this, chCeList, chCeBrLt, (TThing **)tObjList, 10)) {
      sendTo("I'm afraid you don't have all the items you specified or they were of the wrong type.\n\r");
      return 0;
    }

    for (int Runner = 0; Runner < 10; Runner++) {
      delete [] chCeBrLt[Runner];
      chCeBrLt[Runner] = NULL;
    }

    for (int Runner = 5; ((Runner < MAX_APPLIED_SUBSTANCES) && ceLevel == -1); Runner++)
      if (AppliedCreate[Runner] && skLevel >= AppliedCreate[Runner]->skCEMin)
        if (AppliedCreate[Runner]->ceType == ceType)
          if (appliedSubstanceFindMatch((TThing **)tObjList, Runner, 10, CLASS_RANGER))
            ceLevel = Runner;

    if (ceLevel == -1)
      ceLevel = ceType;

    StartCreateEngine(CEM_APPLIED, ceLevel, this, NULL, tArg, (TThing **)tObjList);
  } else
    sendTo("I'm afraid you have no idea how to create anything.\n\r");

  return nRc;
}

void EndCreateEngine(CreateEngineMethods Method, int ceLevel, TBeing *ch, TObj *tObj)
{
  switch (Method) {
    case CEM_APPLIED:
      if (ceLevel < 0 || ceLevel >= MAX_APPLIED_SUBSTANCES) {
        ch->sendTo("Something is wrong, tell a god what you did.\n\r");
        vlogf(LOG_LAPSOS, "Create-Engine(Applied)-End Called with wrong Level.");
        return;
      }

      if (AppliedCreate[ceLevel]->ceEndMsg)
        ch->sendTo(AppliedCreate[ceLevel]->ceEndMsg);

      if (AppliedCreate[ceLevel]->ceEnding)
        AppliedCreate[ceLevel]->ceEnding(ceLevel, ch, tObj);

      ch->stopTask();
      if (ch->act_ptr) {
        delete static_cast<appliedCreate_struct *>(ch->act_ptr);
        ch->act_ptr = NULL;
      } else {
        vlogf(LOG_LAPSOS, "Job was lost while closing Create Engine [CEM_APPLIED]");
        ch->sendTo("Tell a god what you did.\n\r");
      }

      if (AppliedCreate[ceLevel]->CompReward == -1) {
        ch->sendTo("All your work goes for naught, it seems that someone forget to set the item.\n\r");
        vlogf(LOG_LAPSOS, fmt("AppliedCreate[%d]->CompReward is -1, fix please.") %  ceLevel);
        return;
      }

      TObj *FinalObject;
      if (!(FinalObject = read_object(AppliedCreate[ceLevel]->CompReward, REAL))) {
        ch->sendTo("Problem loading the final object, tell a god immediatly.\n\r");
        vlogf(LOG_LAPSOS, fmt("AppliedCreate->CompReward doesn't exist! [%d]") %  AppliedCreate[ceLevel]->CompReward);
        return;
      }

      *ch += *FinalObject;
      break;
    case CEM_BREW:
      break;
    case CEM_ALCHEMIST:
      break;
    default:
      vlogf(LOG_LAPSOS, fmt("EndCreateEngine called with invalid Method [%d]") %  Method);
  }
}

void StartCreateEngine(CreateEngineMethods Method, int ceLevel, TBeing *ch,
                       TObj *tObj, const char *arg, TThing **tObjList)
{
  appliedCreate_struct *asJob;

  switch (Method) {
    case CEM_APPLIED:
      if (ceLevel < 0 || ceLevel >= MAX_APPLIED_SUBSTANCES) {
        ch->sendTo("Something is wrong, tell a god what you did.\n\r");
        vlogf(LOG_LAPSOS, "Create-Engine(Applied) Called with wrong Level.");
        return;
      }
      if (ch->act_ptr) {
        ch->sendTo("Something is preventing you from being able to do this.\n\r");
        return;
      }

      if (!(ch->act_ptr = new appliedCreate_struct())) {
        perror("failed new on applied create.");
        exit(0);
      }

      asJob = static_cast<appliedCreate_struct *>(ch->act_ptr);

      for (int Runner = 0; Runner < 10; Runner++) {
        // The first five are defaults, we Do Not care about there order.
        if (ceLevel < 5) {
          asJob->tObjList[Runner] = tObjList[Runner];
          tObjList[Runner] = NULL;
        } else
          for (int RunnerZ = 0; RunnerZ < 10; RunnerZ++)
            if (tObjList[RunnerZ] && tObj->objVnum() == AppliedCreate[ceLevel]->CompList[Runner]) {
              asJob->tObjList[Runner] = tObjList[RunnerZ];
              tObjList[RunnerZ] = NULL;
            }
      }

      asJob->totMessages = 0;

      if (AppliedCreate[ceLevel]->ceStartMsg)
        ch->sendTo(AppliedCreate[ceLevel]->ceStartMsg);

      if (AppliedCreate[ceLevel]->ceStartup)
        AppliedCreate[ceLevel]->ceStartup(ceLevel, ch, tObj);

      //       0 = timeLeft
      //  Method = status
      // ceLEvel = flag
      start_task(ch, tObj, NULL, TASK_CREATENGINE, arg, 0, ch->in_room, Method, ceLevel, 40);

      break;
    case CEM_BREW:
      break;
    case CEM_ALCHEMIST:
      break;
    default:
      vlogf(LOG_LAPSOS, fmt("StartCreateEngine called with invalid Method [%d]") %  Method);
  }
}

void stop_createTask(TBeing *ch)
{
  ch->sendTo("You stop what you are doing for now.\n\r");
  act("$n stops in the middle of $s work.\n\r",
      FALSE, ch, 0, 0, TO_ROOM);

  if (ch->act_ptr) {
    delete static_cast<appliedCreate_struct *>(ch->act_ptr);
    ch->act_ptr = NULL;
  }

  ch->stopTask();
}

int task_createEngine(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *obj)
{
  int   nRc = 0,
        skLvl,
        skMax,
        skMin,
        skMsgs,
        curMesg,
        curComp,
        sentMessage;
  float skDiff,
        skRnds;
  char  tString[256],
        nameBufs[2][256];

  // basic safty checks:
  if (ch->isLinkdead() || (ch->in_room != ch->task->wasInRoom) ||
      (ch->getPosition() < POSITION_STANDING)) {
    stop_createTask(ch);
    return FALSE;
  }

  if (ch->utilityTaskCommand(cmd) || ch->nobrainerTaskCommand(cmd))
    return FALSE;

  appliedCreate_struct *asJob;

  switch (cmd) {
    case CMD_TASK_CONTINUE:
      switch (ch->task->status) {
        case CEM_APPLIED:
          skMax   = AppliedCreate[ch->task->flags]->skCEMax;
          skMin   = AppliedCreate[ch->task->flags]->skCEMin;
          skLvl   = ch->getSkillValue(AppliedCreate[ch->task->flags]->skCESkillNum);
          skDiff  = ((float)(min(skLvl, skMax) - skMin) / (float)(skMax - skMin));
          skRnds  = ((2.0 - skDiff) * AppliedCreate[ch->task->flags]->skCERounds);
          skMsgs  = (int)(skRnds / 5);
          curMesg = (int)(ch->task->timeLeft / skMsgs);
          curComp = curMesg * 2;

          if (ch->task->timeLeft >= (skRnds + 1)) {
            EndCreateEngine(CEM_APPLIED, ch->task->flags, ch, obj);
            return 0;
          }

          asJob = static_cast<appliedCreate_struct *>(ch->act_ptr);

          if (!asJob) {
            vlogf(LOG_LAPSOS, "Player in the middle of createEngine task (CEM_APPLIED) Without job.");
            ch->sendTo("Something went terribly wrong, tell a coder what you did.\n\r");
            ch->stopTask();
            return 0;
          }

          sentMessage = asJob->totMessages;

          if ((ch->task->timeLeft % skMsgs) == 0 && curMesg < 5 &&
              (sentMessage - 1) < curMesg && AppliedCreate[ch->task->flags]->ceMessages[curMesg]) {

            if ((AppliedCreate[ch->task->flags]->CompList[curComp] != -1 &&
                 !asJob->tObjList[curComp]) ||
                (AppliedCreate[ch->task->flags]->CompList[(curComp + 1)] != -1 &&
                 !asJob->tObjList[(curComp + 1)])) {

              ch->sendTo("For some reason some of your components vanished, how odd...\n\r");
              ch->sendTo("Sadly you cannot continue without those components.\n\r");
              ch->stopTask();
              if (ch->act_ptr) {
                delete static_cast<appliedCreate_struct *>(ch->act_ptr);
                ch->act_ptr = NULL;
              }
              return 0;
            }

            if (AppliedCreate[ch->task->flags]->CompList[curComp] != -1 ||
                (ch->task->flags < 5 && asJob->tObjList[curComp])) {
              --(*asJob->tObjList[curComp]);
              delete asJob->tObjList[curComp];
              asJob->tObjList[curComp] = NULL;
            }

            if (AppliedCreate[ch->task->flags]->CompList[(curComp + 1)] != -1 ||
                (ch->task->flags < 5 && asJob->tObjList[curComp])) {
              --(*asJob->tObjList[(curComp + 1)]);
              delete asJob->tObjList[(curComp + 1)];
              asJob->tObjList[(curComp + 1)] = NULL;
	    }

            if (ch->task->flags < 5 && !AppliedCreate[ch->task->flags]->ceMessages[curMesg]) {
              if (asJob->tObjList[curComp] && asJob->tObjList[(curComp + 1)]) {
                strcpy(nameBufs[0], ch->pers(asJob->tObjList[curComp]));
                strcpy(nameBufs[1], ch->pers(asJob->tObjList[(curComp + 1)]));

                sprintf(tString, "You add the %s and %s to the mixture.",
                        sstring(nameBufs[0]).cap().c_str(),
                        sstring(nameBufs[1]).cap().c_str());
                act(tString, TRUE, ch, 0, 0, TO_CHAR);
              } else if (asJob->tObjList[curComp]) {
                strcpy(nameBufs[0], ch->pers(asJob->tObjList[curComp]));

                sprintf(tString, "You add the %s to the mixture.",
                        sstring(nameBufs[0]).cap().c_str());
                act(tString, TRUE, ch, 0, 0, TO_CHAR);
              } else if (asJob->tObjList[(curComp + 1)]) {
                strcpy(nameBufs[0], ch->pers(asJob->tObjList[(curComp + 1)]));

                sprintf(tString, "You add the %s to the mixture.",
                        sstring(nameBufs[0]).cap().c_str());
                act(tString, TRUE, ch, 0, 0, TO_CHAR);
              }
            } else
              ch->sendTo(AppliedCreate[ch->task->flags]->ceMessages[curMesg]);

            if (AppliedCreate[ch->task->flags]->cePerMesg)
              AppliedCreate[ch->task->flags]->cePerMesg(ch->task->flags, ch, obj);

            asJob->totMessages++;
          }

          switch (AppliedCreate[ch->task->flags]->ceType) {
            case CEMH_AS_BALM:
              nRc = appliedSubstanceCreateBalm(ch, cmd, pulse, obj);
              break;
            case CEMH_AS_SALVE:
              nRc = appliedSubstanceCreateSalve(ch, cmd, pulse, obj);
              break;
            case CEMH_AS_POWDER:
              nRc = appliedSubstanceCreatePowder(ch, cmd, pulse, obj);
              break;
            case CEMH_AS_OIL:
              nRc = appliedSubstanceCreateOil(ch, cmd, pulse, obj);
              break;
            case CEMH_AS_ICHOR:
              nRc = appliedSubstanceCreateIchor(ch, cmd, pulse, obj);
              break;
            default:
              vlogf(LOG_LAPSOS, fmt("task_createEngine/CEM_APPLIED with invalid Level [%d]") %  ch->task->flags);
              ch->sendTo("Something went wrong, tell a coder what you did.\n\r");
              ch->stopTask();
              if (ch->act_ptr) {
                delete static_cast<appliedCreate_struct *>(ch->act_ptr);
                ch->act_ptr = NULL;
              }
          }
          break;
        case CEM_BREW:
          ch->stopTask();
          break;
        case CEM_ALCHEMIST:
          ch->stopTask();
          break;
        default:
          vlogf(LOG_LAPSOS, fmt("task_createEngine called with invalid Method [%d]") %  ch->task->status);
          ch->sendTo("Something went wrong, tell a coder what you did.\n\r");
          if (ch->act_ptr) {
            delete static_cast<appliedCreate_struct *>(ch->act_ptr);
            ch->act_ptr = NULL;
          }
          ch->stopTask();
      }

      return nRc;
      break;
    case CMD_ABORT:
    case CMD_STOP:
      stop_createTask(ch);
      break;
    case CMD_TASK_FIGHTING:
      ch->sendTo("You cannot continue your work under these conditions!\n\r");
      act("$n stops there work with a disgruntled look.",
          FALSE, ch, 0, 0, TO_ROOM);
      if (ch->act_ptr) {
        delete static_cast<appliedCreate_struct *>(ch->act_ptr);
        ch->act_ptr = NULL;
      }
      ch->stopTask();
      break;
    default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);

      break;
  }

  return TRUE;
}
