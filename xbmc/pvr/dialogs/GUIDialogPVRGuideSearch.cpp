/*
 *  Copyright (C) 2012-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "GUIDialogPVRGuideSearch.h"

#include "ServiceBroker.h"
#include "guilib/GUIEditControl.h"
#include "guilib/GUIKeyboardFactory.h"
#include "guilib/GUIMessage.h"
#include "guilib/LocalizeStrings.h"
#include "pvr/PVRManager.h"
#include "pvr/channels/PVRChannel.h"
#include "pvr/channels/PVRChannelGroupMember.h"
#include "pvr/channels/PVRChannelGroups.h"
#include "pvr/channels/PVRChannelGroupsContainer.h"
#include "pvr/epg/EpgContainer.h"
#include "pvr/epg/EpgSearchFilter.h"
#include "utils/StringUtils.h"

#include <cstdio>
#include <string>
#include <utility>
#include <vector>

using namespace PVR;

namespace
{
constexpr unsigned int CONTROL_EDIT_SEARCH = 9;
constexpr unsigned int CONTROL_BTN_INC_DESC = 10;
constexpr unsigned int CONTROL_BTN_CASE_SENS = 11;
constexpr unsigned int CONTROL_SPIN_MIN_DURATION = 12;
constexpr unsigned int CONTROL_SPIN_MAX_DURATION = 13;
constexpr unsigned int CONTROL_EDIT_START_DATE = 14;
constexpr unsigned int CONTROL_EDIT_STOP_DATE = 15;
constexpr unsigned int CONTROL_EDIT_START_TIME = 16;
constexpr unsigned int CONTROL_EDIT_STOP_TIME = 17;
constexpr unsigned int CONTROL_SPIN_GENRE = 18;
constexpr unsigned int CONTROL_SPIN_NO_REPEATS = 19;
constexpr unsigned int CONTROL_BTN_UNK_GENRE = 20;
constexpr unsigned int CONTROL_SPIN_GROUPS = 21;
constexpr unsigned int CONTROL_BTN_FTA_ONLY = 22;
constexpr unsigned int CONTROL_SPIN_CHANNELS = 23;
constexpr unsigned int CONTROL_BTN_IGNORE_TMR = 24;
constexpr unsigned int CONTROL_BTN_CANCEL = 25;
constexpr unsigned int CONTROL_BTN_SEARCH = 26;
constexpr unsigned int CONTROL_BTN_IGNORE_REC = 27;
constexpr unsigned int CONTROL_BTN_DEFAULTS = 28;
constexpr unsigned int CONTROL_BTN_SAVE = 29;
constexpr unsigned int CONTROL_BTN_IGNORE_FINISHED = 30;
constexpr unsigned int CONTROL_BTN_IGNORE_FUTURE = 31;
constexpr unsigned int CONTROL_BTN_START_ANY_TIME = 32;
constexpr unsigned int CONTROL_BTN_END_ANY_TIME = 33;

} // unnamed namespace

CGUIDialogPVRGuideSearch::CGUIDialogPVRGuideSearch()
  : CGUIDialog(WINDOW_DIALOG_PVR_GUIDE_SEARCH, "DialogPVRGuideSearch.xml")
{
}

void CGUIDialogPVRGuideSearch::SetFilterData(
    const std::shared_ptr<CPVREpgSearchFilter>& searchFilter)
{
  m_searchFilter = searchFilter;
}

void CGUIDialogPVRGuideSearch::UpdateChannelSpin()
{
  int iChannelGroup = GetSpinValue(CONTROL_SPIN_GROUPS);

  std::vector< std::pair<std::string, int> > labels;
  if (m_searchFilter->IsRadio())
    labels.emplace_back(g_localizeStrings.Get(19216), EPG_SEARCH_UNSET); // All radio channels
  else
    labels.emplace_back(g_localizeStrings.Get(19217), EPG_SEARCH_UNSET); // All TV channels

  std::shared_ptr<CPVRChannelGroup> group;
  if (iChannelGroup != EPG_SEARCH_UNSET)
    group = CServiceBroker::GetPVRManager().ChannelGroups()->GetByIdFromAll(iChannelGroup);

  if (!group)
    group = CServiceBroker::GetPVRManager().ChannelGroups()->GetGroupAll(m_searchFilter->IsRadio());

  m_channelsMap.clear();
  const std::vector<std::shared_ptr<CPVRChannelGroupMember>> groupMembers =
      group->GetMembers(CPVRChannelGroup::Include::ONLY_VISIBLE);
  int iIndex = 0;
  int iSelectedChannel = EPG_SEARCH_UNSET;
  for (const auto& groupMember : groupMembers)
  {
    labels.emplace_back(groupMember->Channel()->ChannelName(), iIndex);
    m_channelsMap.insert(std::make_pair(iIndex, groupMember));

    if (iSelectedChannel == EPG_SEARCH_UNSET &&
        groupMember->ChannelUID() == m_searchFilter->GetChannelUID() &&
        groupMember->ChannelClientID() == m_searchFilter->GetClientID())
      iSelectedChannel = iIndex;

    ++iIndex;
  }

  SET_CONTROL_LABELS(CONTROL_SPIN_CHANNELS, iSelectedChannel, &labels);
}

void CGUIDialogPVRGuideSearch::UpdateGroupsSpin()
{
  std::vector<std::pair<std::string, int>> labels;
  const std::vector<std::shared_ptr<CPVRChannelGroup>> groups =
      CServiceBroker::GetPVRManager().ChannelGroups()->Get(m_searchFilter->IsRadio())->GetMembers();
  int selectedGroup = EPG_SEARCH_UNSET;
  for (const auto& group : groups)
  {
    labels.emplace_back(group->GroupName(), group->GroupID());

    if (selectedGroup == EPG_SEARCH_UNSET &&
        group->GroupID() == m_searchFilter->GetChannelGroupID())
      selectedGroup = group->GroupID();
  }

  SET_CONTROL_LABELS(CONTROL_SPIN_GROUPS, selectedGroup, &labels);
}

void CGUIDialogPVRGuideSearch::UpdateGenreSpin()
{
  std::vector< std::pair<std::string, int> > labels;
  labels.emplace_back(g_localizeStrings.Get(593), EPG_SEARCH_UNSET);
  labels.emplace_back(g_localizeStrings.Get(19500), EPG_EVENT_CONTENTMASK_MOVIEDRAMA);
  labels.emplace_back(g_localizeStrings.Get(19516), EPG_EVENT_CONTENTMASK_NEWSCURRENTAFFAIRS);
  labels.emplace_back(g_localizeStrings.Get(19532), EPG_EVENT_CONTENTMASK_SHOW);
  labels.emplace_back(g_localizeStrings.Get(19548), EPG_EVENT_CONTENTMASK_SPORTS);
  labels.emplace_back(g_localizeStrings.Get(19564), EPG_EVENT_CONTENTMASK_CHILDRENYOUTH);
  labels.emplace_back(g_localizeStrings.Get(19580), EPG_EVENT_CONTENTMASK_MUSICBALLETDANCE);
  labels.emplace_back(g_localizeStrings.Get(19596), EPG_EVENT_CONTENTMASK_ARTSCULTURE);
  labels.emplace_back(g_localizeStrings.Get(19612), EPG_EVENT_CONTENTMASK_SOCIALPOLITICALECONOMICS);
  labels.emplace_back(g_localizeStrings.Get(19628), EPG_EVENT_CONTENTMASK_EDUCATIONALSCIENCE);
  labels.emplace_back(g_localizeStrings.Get(19644), EPG_EVENT_CONTENTMASK_LEISUREHOBBIES);
  labels.emplace_back(g_localizeStrings.Get(19660), EPG_EVENT_CONTENTMASK_SPECIAL);
  labels.emplace_back(g_localizeStrings.Get(19499), EPG_EVENT_CONTENTMASK_USERDEFINED);

  SET_CONTROL_LABELS(CONTROL_SPIN_GENRE, m_searchFilter->GetGenreType(), &labels);
}

void CGUIDialogPVRGuideSearch::UpdateDurationSpin()
{
  /* minimum duration */
  std::vector< std::pair<std::string, int> > labels;

  labels.emplace_back("-", EPG_SEARCH_UNSET);
  for (int i = 1; i < 12*60/5; ++i)
    labels.emplace_back(StringUtils::Format(g_localizeStrings.Get(14044), i * 5), i * 5);

  SET_CONTROL_LABELS(CONTROL_SPIN_MIN_DURATION, m_searchFilter->GetMinimumDuration(), &labels);

  /* maximum duration */
  labels.clear();

  labels.emplace_back("-", EPG_SEARCH_UNSET);
  for (int i = 1; i < 12*60/5; ++i)
    labels.emplace_back(StringUtils::Format(g_localizeStrings.Get(14044), i * 5), i * 5);

  SET_CONTROL_LABELS(CONTROL_SPIN_MAX_DURATION, m_searchFilter->GetMaximumDuration(), &labels);
}

bool CGUIDialogPVRGuideSearch::OnMessage(CGUIMessage& message)
{
  CGUIDialog::OnMessage(message);

  if (message.GetMessage() == GUI_MSG_CLICKED)
  {
    switch (message.GetSenderId())
    {
      case CONTROL_BTN_SEARCH:
      {
        // Read data from controls, update m_searchfilter accordingly
        UpdateSearchFilter();

        m_result = Result::SEARCH;
        Close();
        return true;
      }
      case CONTROL_BTN_CANCEL:
      {
        m_result = Result::CANCEL;
        Close();
        return true;
      }
      case CONTROL_BTN_DEFAULTS:
      {
        if (m_searchFilter)
        {
          m_searchFilter->Reset();
          Update();
        }
        return true;
      }
      case CONTROL_BTN_SAVE:
      {
        // Read data from controls, update m_searchfilter accordingly
        UpdateSearchFilter();

        std::string title{m_searchFilter->GetTitle()};
        if (title.empty())
        {
          title = m_searchFilter->GetSearchTerm();
          if (title.empty())
            title = g_localizeStrings.Get(137); // "Search"
          else
            StringUtils::Trim(title, "\"");

          if (!CGUIKeyboardFactory::ShowAndGetInput(
                  title, CVariant{g_localizeStrings.Get(528)}, // "Enter title"
                  false))
          {
            return false;
          }
          m_searchFilter->SetTitle(title);
        }

        m_result = Result::SAVE;
        Close();
        return true;
      }
      case CONTROL_SPIN_GROUPS:
      {
        UpdateChannelSpin();
        return true;
      }
      case CONTROL_BTN_START_ANY_TIME:
      case CONTROL_BTN_END_ANY_TIME:
      {
        UpdateSearchFilter();
        Update();
        return true;
      }
      default:
        break;
    }
  }
  return false;
}

void CGUIDialogPVRGuideSearch::OnInitWindow()
{
  CGUIDialog::OnInitWindow();

  m_result = Result::CANCEL;
}

void CGUIDialogPVRGuideSearch::OnWindowLoaded()
{
  Update();
  return CGUIDialog::OnWindowLoaded();
}

CDateTime CGUIDialogPVRGuideSearch::ReadDateTime(const std::string& strDate, const std::string& strTime) const
{
  CDateTime dateTime;
  int iHours{0};
  int iMinutes{0};
  std::sscanf(strTime.c_str(), "%d:%d", &iHours, &iMinutes);
  dateTime.SetFromDBDate(strDate);
  dateTime.SetDateTime(dateTime.GetYear(), dateTime.GetMonth(), dateTime.GetDay(), iHours, iMinutes, 0);
  return dateTime.GetAsUTCDateTime();
}

bool CGUIDialogPVRGuideSearch::IsRadioSelected(int controlID)
{
  CGUIMessage msg(GUI_MSG_IS_SELECTED, GetID(), controlID);
  OnMessage(msg);
  return (msg.GetParam1() == 1);
}

int CGUIDialogPVRGuideSearch::GetSpinValue(int controlID)
{
  CGUIMessage msg(GUI_MSG_ITEM_SELECTED, GetID(), controlID);
  OnMessage(msg);
  return msg.GetParam1();
}

std::string CGUIDialogPVRGuideSearch::GetEditValue(int controlID)
{
  CGUIMessage msg(GUI_MSG_ITEM_SELECTED, GetID(), controlID);
  OnMessage(msg);
  return msg.GetLabel();
}

void CGUIDialogPVRGuideSearch::UpdateSearchFilter()
{
  if (!m_searchFilter)
    return;

  m_searchFilter->SetSearchTerm(GetEditValue(CONTROL_EDIT_SEARCH));

  m_searchFilter->SetSearchInDescription(IsRadioSelected(CONTROL_BTN_INC_DESC));
  m_searchFilter->SetCaseSensitive(IsRadioSelected(CONTROL_BTN_CASE_SENS));
  m_searchFilter->SetFreeToAirOnly(IsRadioSelected(CONTROL_BTN_FTA_ONLY));
  m_searchFilter->SetIncludeUnknownGenres(IsRadioSelected(CONTROL_BTN_UNK_GENRE));
  m_searchFilter->SetIgnorePresentRecordings(IsRadioSelected(CONTROL_BTN_IGNORE_REC));
  m_searchFilter->SetIgnorePresentTimers(IsRadioSelected(CONTROL_BTN_IGNORE_TMR));
  m_searchFilter->SetRemoveDuplicates(IsRadioSelected(CONTROL_SPIN_NO_REPEATS));
  m_searchFilter->SetIgnoreFinishedBroadcasts(IsRadioSelected(CONTROL_BTN_IGNORE_FINISHED));
  m_searchFilter->SetIgnoreFutureBroadcasts(IsRadioSelected(CONTROL_BTN_IGNORE_FUTURE));
  m_searchFilter->SetGenreType(GetSpinValue(CONTROL_SPIN_GENRE));
  m_searchFilter->SetMinimumDuration(GetSpinValue(CONTROL_SPIN_MIN_DURATION));
  m_searchFilter->SetMaximumDuration(GetSpinValue(CONTROL_SPIN_MAX_DURATION));

  auto it = m_channelsMap.find(GetSpinValue(CONTROL_SPIN_CHANNELS));
  m_searchFilter->SetClientID(it == m_channelsMap.end() ? PVR_CLIENT_INVALID_UID
                                                        : (*it).second->ChannelClientID());
  m_searchFilter->SetChannelUID(it == m_channelsMap.end() ? -1 : (*it).second->ChannelUID());
  m_searchFilter->SetChannelGroupID(GetSpinValue(CONTROL_SPIN_GROUPS));

  const CDateTime start =
      ReadDateTime(GetEditValue(CONTROL_EDIT_START_DATE), GetEditValue(CONTROL_EDIT_START_TIME));
  if (start != m_startDateTime)
  {
    m_searchFilter->SetStartDateTime(start);
    m_startDateTime = start;
  }
  const CDateTime end =
      ReadDateTime(GetEditValue(CONTROL_EDIT_STOP_DATE), GetEditValue(CONTROL_EDIT_STOP_TIME));
  if (end != m_endDateTime)
  {
    m_searchFilter->SetEndDateTime(end);
    m_endDateTime = end;
  }

  m_searchFilter->SetStartAnyTime(IsRadioSelected(CONTROL_BTN_START_ANY_TIME));
  m_searchFilter->SetEndAnyTime(IsRadioSelected(CONTROL_BTN_END_ANY_TIME));
}

void CGUIDialogPVRGuideSearch::Update()
{
  if (!m_searchFilter)
    return;

  SET_CONTROL_LABEL2(CONTROL_EDIT_SEARCH, m_searchFilter->GetSearchTerm());
  {
    CGUIMessage msg(GUI_MSG_SET_TYPE, GetID(), CONTROL_EDIT_SEARCH, CGUIEditControl::INPUT_TYPE_TEXT, 16017);
    OnMessage(msg);
  }

  SET_CONTROL_SELECTED(GetID(), CONTROL_BTN_CASE_SENS, m_searchFilter->IsCaseSensitive());
  SET_CONTROL_SELECTED(GetID(), CONTROL_BTN_INC_DESC, m_searchFilter->ShouldSearchInDescription());
  SET_CONTROL_SELECTED(GetID(), CONTROL_BTN_FTA_ONLY, m_searchFilter->IsFreeToAirOnly());
  SET_CONTROL_SELECTED(GetID(), CONTROL_BTN_UNK_GENRE, m_searchFilter->ShouldIncludeUnknownGenres());
  SET_CONTROL_SELECTED(GetID(), CONTROL_BTN_IGNORE_REC, m_searchFilter->ShouldIgnorePresentRecordings());
  SET_CONTROL_SELECTED(GetID(), CONTROL_BTN_IGNORE_TMR, m_searchFilter->ShouldIgnorePresentTimers());
  SET_CONTROL_SELECTED(GetID(), CONTROL_SPIN_NO_REPEATS, m_searchFilter->ShouldRemoveDuplicates());
  SET_CONTROL_SELECTED(GetID(), CONTROL_BTN_IGNORE_FINISHED,
                       m_searchFilter->ShouldIgnoreFinishedBroadcasts());
  SET_CONTROL_SELECTED(GetID(), CONTROL_BTN_IGNORE_FUTURE,
                       m_searchFilter->ShouldIgnoreFutureBroadcasts());
  SET_CONTROL_SELECTED(GetID(), CONTROL_BTN_START_ANY_TIME, m_searchFilter->IsStartAnyTime());
  SET_CONTROL_SELECTED(GetID(), CONTROL_BTN_END_ANY_TIME, m_searchFilter->IsEndAnyTime());
  CONTROL_ENABLE_ON_CONDITION(CONTROL_EDIT_START_TIME, !m_searchFilter->IsStartAnyTime());
  CONTROL_ENABLE_ON_CONDITION(CONTROL_EDIT_STOP_TIME, !m_searchFilter->IsEndAnyTime());

  // Set start/end datetime fields
  m_startDateTime = m_searchFilter->GetStartDateTime();
  m_endDateTime = m_searchFilter->GetEndDateTime();
  if (!m_startDateTime.IsValid() || !m_endDateTime.IsValid())
  {
    const auto [first, last] =
        CServiceBroker::GetPVRManager().EpgContainer().GetFirstAndLastEPGDate();
    if (!m_startDateTime.IsValid())
      m_startDateTime = first;
    if (!m_endDateTime.IsValid())
      m_endDateTime = last;
  }

  if (!m_startDateTime.IsValid())
    m_startDateTime = CDateTime::GetUTCDateTime();

  if (!m_endDateTime.IsValid())
    m_endDateTime = m_startDateTime + CDateTimeSpan(10, 0, 0, 0); // default to start + 10 days

  CDateTime startLocal;
  startLocal.SetFromUTCDateTime(m_startDateTime);
  CDateTime endLocal;
  endLocal.SetFromUTCDateTime(m_endDateTime);

  SET_CONTROL_LABEL2(CONTROL_EDIT_START_TIME, startLocal.GetAsLocalizedTime("", false));
  {
    CGUIMessage msg(GUI_MSG_SET_TYPE, GetID(), CONTROL_EDIT_START_TIME, CGUIEditControl::INPUT_TYPE_TIME, 14066);
    OnMessage(msg);
  }
  SET_CONTROL_LABEL2(CONTROL_EDIT_STOP_TIME, endLocal.GetAsLocalizedTime("", false));
  {
    CGUIMessage msg(GUI_MSG_SET_TYPE, GetID(), CONTROL_EDIT_STOP_TIME, CGUIEditControl::INPUT_TYPE_TIME, 14066);
    OnMessage(msg);
  }
  SET_CONTROL_LABEL2(CONTROL_EDIT_START_DATE, startLocal.GetAsDBDate());
  {
    CGUIMessage msg(GUI_MSG_SET_TYPE, GetID(), CONTROL_EDIT_START_DATE, CGUIEditControl::INPUT_TYPE_DATE, 14067);
    OnMessage(msg);
  }
  SET_CONTROL_LABEL2(CONTROL_EDIT_STOP_DATE, endLocal.GetAsDBDate());
  {
    CGUIMessage msg(GUI_MSG_SET_TYPE, GetID(), CONTROL_EDIT_STOP_DATE, CGUIEditControl::INPUT_TYPE_DATE, 14067);
    OnMessage(msg);
  }

  UpdateDurationSpin();
  UpdateGroupsSpin();
  UpdateChannelSpin();
  UpdateGenreSpin();
}
