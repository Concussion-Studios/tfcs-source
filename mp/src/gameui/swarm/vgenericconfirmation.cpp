//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "VGenericConfirmation.h"
#include "vgui_controls/Label.h"
#include "vgui/ISurface.h"
#include "ExMenuButton.h"
#include "cdll_util.h"
#include <vgui_controls/AnimationController.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

int GenericConfirmation::sm_currentUsageId = 0;

GenericConfirmation::Data_t::Data_t() : 
	pWindowTitle( NULL ),
	pMessageText( NULL ),
	pMessageTextW( NULL ),
	bOkButtonEnabled( false ),
	pfnOkCallback( NULL ),
	bCancelButtonEnabled( false ),
	pfnCancelCallback( NULL ),
	bCheckBoxEnabled( false ),
	pCheckBoxLabelText( NULL ),
	pCheckBoxCvarName( NULL )
{
}

//=============================================================================
GenericConfirmation::GenericConfirmation( Panel *parent, const char *panelName ):
	BaseClass( parent, panelName, true, true, false ),
	m_pLblMessage( 0 ),
	m_pLblCheckBox( 0 ),
	m_data(),
	m_usageId( 0 ),
	m_pCheckBox( 0 )
{
	SetProportional( true );

	m_pPnlLowerGarnish = new vgui::Panel( this, "PnlLowerGarnish" );

	m_pBtnOK = new CExMenuButton( this, "BtnOK", "", this, "OK" );
	m_pBtnCancel = new CExMenuButton( this, "BtnCancel", "", this, "cancel" );

	SetTitle( "", false );
	SetDeleteSelfOnClose( true );
	SetLowerGarnishEnabled( false );
	SetMoveable( false );
}

//=============================================================================
GenericConfirmation::~GenericConfirmation()
{
	delete m_pLblMessage;
	delete m_pPnlLowerGarnish;
}

//=============================================================================
void GenericConfirmation::OnCommand(const char *command)
{
	if ( Q_stricmp( command, "OK" ) == 0 )
	{
		OnKeyCodePressed( ButtonCodeToJoystickButtonCode( KEY_XBUTTON_A, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) );
	}
	else if ( Q_stricmp( command, "cancel" ) == 0 )
	{
		OnKeyCodePressed( ButtonCodeToJoystickButtonCode( KEY_XBUTTON_B, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) );
	}
}

//=============================================================================
void GenericConfirmation::OnKeyCodePressed( KeyCode keycode )
{
	int userId = GetJoystickForCode( keycode );
	vgui::KeyCode code = GetBaseButtonCode( keycode );
	BaseModUI::CBaseModPanel::GetSingleton().SetLastActiveUserId( userId );

	switch ( code )
	{
	case KEY_XBUTTON_A:
		if ( m_OkButtonEnabled )
		{
			if ( m_pCheckBox )
			{
				m_pCheckBox->ApplyChanges();
			}

			CBaseModPanel::GetSingleton().PlayUISound( UISOUND_ACCEPT );
			if ( !NavigateBack() )
			{
				Close();
			}

			if ( m_data.pfnOkCallback != 0 )
			{
				m_data.pfnOkCallback();
			}
		}
		break;

	case KEY_XBUTTON_B:
		if ( m_CancelButtonEnabled )
		{
			if ( m_pCheckBox )
			{
				m_pCheckBox->ApplyChanges();
			}

			CBaseModPanel::GetSingleton().PlayUISound( UISOUND_BACK );
			if ( !NavigateBack() )
			{
				Close();
			}

			if ( m_data.pfnCancelCallback != 0 )
			{
				m_data.pfnCancelCallback();
			}
		}
		break;
	default:
		BaseClass::OnKeyCodePressed(keycode);
		break;
	}
}

void GenericConfirmation::OnKeyCodeTyped( vgui::KeyCode code )
{
	// For PC, this maps space bar and enter to OK and esc to cancel
	switch ( code )
	{
	case KEY_SPACE:
	case KEY_ENTER:
		return OnKeyCodePressed( ButtonCodeToJoystickButtonCode( KEY_XBUTTON_A, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) );

	case KEY_ESCAPE:
		return OnKeyCodePressed( ButtonCodeToJoystickButtonCode( KEY_XBUTTON_B, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) );
	}

	BaseClass::OnKeyTyped( code );
}

void GenericConfirmation::OnOpen( )
{
	BaseClass::OnOpen();

	m_bNeedsMoveToFront = true;
}

void ExpandButtonWidthIfNecessary( CExMenuButton *pButton )
{
	int originalWide, originalTall;
	pButton->GetSize( originalWide, originalTall );

	pButton->SizeToContents();

	// restore tall
	pButton->SetTall( originalTall );

	const int iMinButtonWidth = MAX( originalWide, scheme()->GetProportionalScaledValue( 45 ) );

	// if we're smaller than original wide, restore wide
	if ( pButton->GetWide() < iMinButtonWidth )
	{
		pButton->SetWide( originalWide );
	}
}

//=============================================================================
void GenericConfirmation::LoadLayout()
{
	BaseClass::LoadLayout();

	int screenWidth, screenHeight;
	CBaseModPanel::GetSingleton().GetSize( screenWidth, screenHeight );

	int dialogWidth = ScreenWidth() * 0.3; //vgui::scheme()->GetProportionalScaledValueEx( GetScheme(), 100 );
	int dialogHeight = vgui::scheme()->GetProportionalScaledValueEx( GetScheme(), 66 );

	// need a border gap to inset all controls
	int borderGap = vgui::scheme()->GetProportionalScaledValueEx( GetScheme(), 8 );

	int checkBoxGap = 0;

	// first solve the size of the parent window
	int titleWide = 0;
	int titleTall = 0;
	vgui::Label *pLblTitle = dynamic_cast< vgui::Label* >( FindChildByName( "LblTitle" ) );
	if ( pLblTitle )
	{
		// account for size of the title and a gap
		pLblTitle->GetContentSize( titleWide, titleTall );
		int shim = vgui::scheme()->GetProportionalScaledValueEx( GetScheme(), IsPC() ? 90 : 15 );
		if ( dialogWidth < titleWide + shim )
		{
			dialogWidth = titleWide + shim;
		}
	}

	if ( m_pLblMessage )
	{
		// account for the size of the message and a gap
		int msgWide, msgTall;
		m_pLblMessage->GetContentSize( msgWide, msgTall );

		if ( msgWide > screenWidth - 100 )
		{
			m_pLblMessage->SetWrap( true );
			m_pLblMessage->SetWide( screenWidth - 100 );
			m_pLblMessage->SetTextInset( 0, 0 );
			m_pLblMessage->GetContentSize( msgWide, msgTall );
		}

		int shimX = vgui::scheme()->GetProportionalScaledValueEx( GetScheme(), IsPC() ? 80 : 20 );
		int shimY = vgui::scheme()->GetProportionalScaledValueEx( GetScheme(), 50 );
		if ( dialogWidth < msgWide + shimX )
		{
			dialogWidth = msgWide + shimX;
		}
		if ( dialogHeight < msgTall + shimY )
		{
			dialogHeight = msgTall + shimY;
		}

		if ( m_pCheckBox )
		{
			int boxWide, boxTall;
			m_pCheckBox->GetContentSize( boxWide, boxTall );
			checkBoxGap = boxTall * 2;
			dialogHeight += checkBoxGap;
		}
	}

	int buttonWide = 0;
	int buttonTall = 0;

	// On the PC, the buttons will be the same size, use the OK button
	CExMenuButton *pOkButton = NULL;
	CExMenuButton *pCancelButton = NULL;
	if ( IsPC() )
	{
		pOkButton = dynamic_cast< CExMenuButton* >( FindChildByName( "BtnOk" ) );
		pCancelButton = dynamic_cast< CExMenuButton* >( FindChildByName( "BtnCancel" ) );
		pOkButton->GetSize( buttonWide, buttonTall );
	}

	if ( m_data.bOkButtonEnabled || m_data.bCancelButtonEnabled )
	{
		// account for the buttons
		dialogHeight += vgui::scheme()->GetProportionalScaledValueEx( GetScheme(), buttonTall );
		dialogHeight += borderGap;
	}

	// don't get smaller than a minimum width
	int minWidth = vgui::scheme()->GetProportionalScaledValueEx( GetScheme(), 235 );
	if ( dialogWidth < minWidth )
		dialogWidth = minWidth;

	// now have final dialog dimensions, center the dialog
	SetPos( ( screenWidth - dialogWidth ) / 2, ( screenHeight - dialogHeight ) / 2 );
	SetSize( dialogWidth, dialogHeight );

	if ( pLblTitle )
	{
		// horizontally center and vertically inset the title
		pLblTitle->SetPos( ( dialogWidth - titleWide ) / 2, borderGap );
		pLblTitle->SetSize( titleWide, titleTall );
	}

	if ( m_pLblMessage )
	{
		// center the message
		int msgWide, msgTall;
		m_pLblMessage->GetContentSize( msgWide, msgTall );
		m_pLblMessage->SetPos( ( dialogWidth - msgWide ) / 2, ( dialogHeight - checkBoxGap - msgTall ) / 2 );
		m_pLblMessage->SetSize( msgWide, msgTall );

		if ( m_pCheckBox )
		{
			int boxWide, boxTall;
			m_pCheckBox->GetContentSize( boxWide, boxTall );
			m_pCheckBox->SetVisible( true );

			int boxX = ( dialogWidth - boxWide ) / 2;
			int boxY = ( dialogHeight - checkBoxGap - msgTall ) / 2 + msgTall + checkBoxGap / 2;

			if ( m_pLblCheckBox )
			{
				int lblCheckBoxWide, lblCheckBoxTall;
				m_pLblCheckBox->GetContentSize( lblCheckBoxWide, lblCheckBoxTall );
				boxX = ( dialogWidth - ( boxWide + lblCheckBoxWide ) ) / 2;

				m_pLblCheckBox->SetVisible( true );
				m_pLblCheckBox->SetPos( boxX + boxWide, boxY );
				m_pLblCheckBox->SetSize( lblCheckBoxWide, lblCheckBoxTall );
			}
			
			m_pCheckBox->SetPos( boxX, boxY );
		}

	}

	if ( IsPC() )
	{
		if ( pOkButton )
		{
			pOkButton->SetVisible( m_data.bOkButtonEnabled );
			ExpandButtonWidthIfNecessary( pOkButton );
		}
		if ( pCancelButton )
		{
			pCancelButton->SetVisible( m_data.bCancelButtonEnabled );
			ExpandButtonWidthIfNecessary( pCancelButton );
		}

		if ( m_data.bCancelButtonEnabled || m_data.bOkButtonEnabled )
		{
			// when only one button is enabled, center that button
			CExMenuButton *pButton = NULL;
			bool bSingleButton = false;
			if ( ( m_data.bCancelButtonEnabled && !m_data.bOkButtonEnabled ) )
			{
				// cancel is centered
				bSingleButton = true;
				pButton = pCancelButton;
			}
			else if ( !m_data.bCancelButtonEnabled && m_data.bOkButtonEnabled )
			{
				// OK is centered
				bSingleButton = true;
				pButton = pOkButton;
			}

			if ( bSingleButton )
			{
				// center the button
				pButton->SetPos( ( dialogWidth - pButton->GetWide() )/2, dialogHeight - borderGap - buttonTall );
			}
			else
			{
				// center left the OK
				pOkButton->SetPos( dialogWidth/2 - buttonWide - borderGap/2, dialogHeight - borderGap - buttonTall );
				
				// center right the CANCEL
				pCancelButton->SetPos( dialogWidth/2 + borderGap/2, dialogHeight - borderGap - buttonTall );
			}
		}
	}
}

//=============================================================================
void GenericConfirmation::PaintBackground()
{
	BaseClass::PaintBackground();

	if ( m_bNeedsMoveToFront )
	{
		vgui::ipanel()->MoveToFront( GetVPanel() );
		m_bNeedsMoveToFront = false;
	}
}

//=============================================================================
// returns the usageId, which gets incremented each time this function is called
int GenericConfirmation::SetUsageData( const Data_t & data )
{
	m_data = data;

	SetTitle( data.pWindowTitle, false );

	if ( m_pLblMessage )
	{
		m_pLblMessage->DeletePanel();
		m_pLblMessage = NULL;
	}

	if ( data.pMessageTextW )
		m_pLblMessage = new Label( this, "LblMessage", data.pMessageTextW );
	else
		m_pLblMessage = new Label( this, "LblMessage", data.pMessageText );

	// tell our base version so input is disabled
	m_OkButtonEnabled = data.bOkButtonEnabled;
	m_CancelButtonEnabled = data.bCancelButtonEnabled;

	if ( data.bCheckBoxEnabled && data.pCheckBoxCvarName )
	{
		m_pCheckBox = new CvarToggleCheckButton<ConVarRef>( 
			this, 
			"CheckButton", 
			"",
			data.pCheckBoxCvarName,
			true );

		if ( m_pLblCheckBox )
		{
			m_pLblCheckBox->DeletePanel();
			m_pLblCheckBox = NULL;
		}

		m_pLblCheckBox = new Label( this, "LblCheckButton", data.pCheckBoxLabelText );
	}

	if ( UI_IsDebug() )
	{
		ConColorMsg( Color( 77, 116, 85, 255 ),  "[GAMEUI] GenericConfirmation::SetWindowTitle : %s\n", data.pWindowTitle!=NULL ? data.pWindowTitle : "<NULL>" );
		ConColorMsg( Color( 77, 116, 85, 255 ),  "[GAMEUI] GenericConfirmation::SetMessageText : %s\n", data.pMessageText!=NULL ? data.pMessageText : "<NULL>" );
	}

	// the window may need to be resized.
	LoadLayout();

	return m_usageId = ++sm_currentUsageId;
}


//=============================================================================
void GenericConfirmation::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	LoadLayout();

	//
	// Override title and msg font
	//
	m_hTitleFont = pScheme->GetFont( "MainBold", true );
	m_hMessageFont = pScheme->GetFont( "Default", true );

	if ( m_LblTitle )
	{
		m_LblTitle->SetFont( m_hTitleFont );
	}

	if ( m_pLblMessage )
	{
		m_pLblMessage->SetFont( m_hMessageFont );
	}

	if ( m_pLblCheckBox )
	{
		m_pLblCheckBox->SetFont( m_hMessageFont );
	}
}
