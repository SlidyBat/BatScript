#pragma once

#include "bat_callable.h"

namespace Bat
{

class BatModule
{
public:
	void AddNative( std::string name, BatNativeCallback callback )
    {
        Native native;
        native.name = std::move( name );
        native.callback = std::move( callback );

        m_Natives.push_back( std::move( native ) );
    }
    const BatNativeCallback* FindNative( const std::string& name ) const
    {
        for( const Native& native : m_Natives )
        {
            if( native.name == name )
                return &native.callback;
        }
        
        return nullptr;
    }
private:
    struct Native
    {
        std::string name;
        BatNativeCallback callback;
    };
    std::vector<Native> m_Natives;
};

}